"""Python model of the 8-bit movement rules as implemented in src/ (Player, PhysicsManager, MapLoader, Game's camera).

Everything is integer: positions and speeds in 1/256 pixel ("sub"). The C++ is written to do exactly the same
operations in the same order; what this prints is what the C++ is expected to do. The programs built from the C++
are never run on this PC (Defender blocks them): they run in the GitHub workflow.

    python model_ref.py figures                  the derived figures of the reference, as a check of the model
    python model_ref.py run <repo> --frames N [--hold keys[:n]]... [--trace every]
"""
import sys
from pathlib import Path

SUB = 256
TILE = 32
RES_W, RES_H = 256, 192

# ---- constants (raw values of the reference, in sub) --------------------------------------------
TOP_SPEED, ROLL_TOP_SPEED = 1024, 1536
ACCELERATION, SKID_BRAKING, FRICTION, ROLL_FRICTION = 16, 32, 20, 5
ROLL_FORWARD, ROLL_BACK = 4, 16
LOW_SPEED, STOP_SPEED, ROLL_STOP_SPEED, MIN_ROLL_SPEED, MIN_SKID_SPEED = 128, 32, 16, 256, 256
SLOPE_PUSH = {1: 31, 2: 24, 4: 16}
GROUND_STICK = 1792
JUMP_SPEED, JUMP_HOLD_UPDATES = 1088, 14
GRAVITY, RAMP_GRAVITY, MAX_FALL_SPEED = 48, 36, 1792
AIR_ACCELERATION, AIR_BRAKING, AIR_DRAG = 16, 64, 16
WALK_OFF_SPEED, CEILING_BUMP_SPEED = 256, 256
HURT_SPEED_X, HURT_SPEED_Y, HURT_UPDATES = 256, 1024, 120
WALL_PROBE_X, WALL_PROBE_Y, CEILING_PROBE_Y = 10, 12, 24
ONE_WAY_REACH = 9
BOX_HALF_WIDTH, BOX_HEIGHT, BALL_HEIGHT = 4, 28, 18
SCREEN_LEFT, SCREEN_RIGHT = 16, 247
CAM_CENTRE_X, CAM_WINDOW_X, CAM_WINDOW_Y, CAM_MAX_STEP, CAM_QUIRK_STEP = 128, 8, 16, 8, 9
CAM_LINE_START, CAM_LINE, CAM_LINE_LOOK_UP, CAM_LINE_CROUCH = 120, 128, 184, 48
CLOCK_UPDATES_PER_SECOND = 60

STANDING, WALKING, SKIDDING, ROLLING, CROUCHING, LOOKING_UP, JUMPING, FALLING, RAMP_JUMPING, HURT = range(10)
NAMES = ["standing", "walking", "skidding", "rolling", "crouching", "looking up", "jumping", "falling", "ramp jump", "hurt"]
AIR = (JUMPING, FALLING, RAMP_JUMPING, HURT)
EMPTY, SOLID, ONE_WAY = 0, 1, 2


def pixel(sub):
    return sub // SUB  # floor


def set_pixel(sub, whole):
    """A correction moves the whole pixels only: the fraction is kept."""
    return whole * SUB + (sub % SUB)


def lround(value):
    import math
    return int(math.floor(value + 0.5)) if value >= 0 else -int(math.floor(-value + 0.5))


class Level:
    def __init__(self, tiles_text, level_text):
        self.kinds = {}
        for line in tiles_text.splitlines():
            if not line or line[0] == "#":
                continue
            words = line.split()
            if len(words) < 3:
                continue
            kind = {"name": words[1], "type": EMPTY, "heights": [0] * TILE, "push": 0}
            if words[2] == "solid":
                kind["type"] = SOLID
                kind["heights"] = [TILE] * TILE
            elif words[2] == "oneway":
                kind["type"] = ONE_WAY
                kind["heights"] = [TILE] * TILE
            elif words[2] == "slope":
                kind["type"] = SOLID
                left, right = int(words[3]), int(words[4])
                # float32 arithmetic in the C++; the values here are exact in both
                kind["heights"] = [min(TILE, max(0, TILE - lround(left + (right - left) * c / (TILE - 1)))) for c in range(TILE)]
                rise = kind["heights"][TILE - 1] - kind["heights"][0]
                if len(words) > 5:
                    steep = {"1:1": 1, "1:2": 2, "1:4": 4}[words[5]]
                else:
                    steep = 1 if abs(rise) >= 24 else 2 if abs(rise) >= 12 else 4
                # the push is downhill: to the left when the ground rises to the right
                kind["push"] = 0 if rise == 0 else (-SLOPE_PUSH[steep] if rise > 0 else SLOPE_PUSH[steep])
            self.kinds[words[0]] = kind
        self.rows, self.pickups, self.width, self.start = [], [], 0, (64, 64)
        for line in level_text.splitlines():
            line = list(line.rstrip("\r"))
            y = len(self.rows)
            for x, ch in enumerate(line):
                if ch == "P":
                    self.start = (x * TILE, y * TILE)
                    line[x] = "."
                elif ch == "o":
                    self.pickups.append((x * TILE + TILE // 4, y * TILE + TILE // 4))
                    line[x] = "."
            self.width = max(self.width, len(line))
            self.rows.append(line)
        for row in self.rows:
            row.extend("." * (self.width - len(row)))

    def kind_at(self, xb, yb):
        if yb < 0 or yb >= len(self.rows) or xb < 0 or xb >= self.width:
            return None
        return self.kinds.get(self.rows[yb][xb])

    def is_edge(self, xb, yb):
        return yb + 2 > len(self.rows) or xb < 0 or xb >= self.width

    def block_type(self, xb, yb):
        """MapLoader::GetBlockType: the bottom row and what is outside the sides is solid, above is air."""
        if self.is_edge(xb, yb):
            return SOLID
        if yb < 0:
            return EMPTY
        kind = self.kind_at(xb, yb)
        return EMPTY if kind is None else kind["type"]

    def ground_height(self, xb, yb, column):
        if self.is_edge(xb, yb):
            return TILE
        kind = self.kind_at(xb, yb)
        return 0 if kind is None else kind["heights"][column]

    def slope_push(self, xb, yb):
        if self.is_edge(xb, yb):
            return 0
        kind = self.kind_at(xb, yb)
        return 0 if kind is None else kind["push"]

    def is_solid(self, x, y):
        """PhysicsManager::IsSolid: inside fully solid ground (what walls and ceilings are made of)."""
        xb, yb = x // TILE, y // TILE
        if self.block_type(xb, yb) != SOLID:
            return False
        return y - yb * TILE >= TILE - self.ground_height(xb, yb, x - xb * TILE)


class Player:
    def __init__(self, game):
        self.game = game
        self.x = self.y = 0  # sub; the middle of the feet
        self.vx = self.vy = 0
        self.state = FALLING
        self.cap = TOP_SPEED
        self.on_floor = False
        self.wall_left = self.wall_right = False
        self.slope = 0
        self.skid_dir = 0
        self.facing = 1
        self.jump_counter, self.jump_boost = 0, False
        self.jump_held = self.jump_was_held = False
        self.left = self.right = self.up = self.down = False
        self.immortal = 0

    def px(self):
        return pixel(self.x)

    def py(self):
        return pixel(self.y)

    def is_ball(self):
        return self.state in (ROLLING, JUMPING, RAMP_JUMPING)

    def input_term(self, direction):
        vx, state = self.vx, self.state
        moving = (vx > 0) - (vx < 0)
        term = 0
        friction = 0
        if state in (STANDING, CROUCHING, LOOKING_UP):
            if direction == 0:
                friction = FRICTION
        elif state == WALKING:
            if direction != 0:
                term = ACCELERATION * direction
            else:
                friction = FRICTION
        elif state == SKIDDING:
            if direction == 0:
                friction = FRICTION
            elif direction == -moving:
                term = SKID_BRAKING * direction
        elif state == ROLLING:
            if direction == 0:
                friction = ROLL_FRICTION
            elif direction == moving:
                term = -ROLL_FORWARD * moving
            elif direction == -moving:
                term = -ROLL_BACK * moving
        elif state in (JUMPING, RAMP_JUMPING):
            if direction == 0:
                friction = AIR_DRAG
            elif direction == -moving:
                term = AIR_BRAKING * direction
            else:
                term = AIR_ACCELERATION * direction
        elif state == FALLING:
            if direction == 0:
                friction = AIR_DRAG
            else:
                term = AIR_ACCELERATION * direction
        term -= friction * moving
        if abs(vx) < LOW_SPEED:
            term *= 2
        # In the air the drag stops at zero (the reference lets it overshoot and oscillate: a quirk not copied).
        if friction and state in AIR and abs(term) > abs(vx):
            term = -vx
        return term

    def update(self):
        game, level = self.game, self.game.level
        jump_pressed = self.jump_held and not self.jump_was_held
        self.jump_was_held = self.jump_held
        if self.immortal > 0:
            self.immortal -= 1

        # 1. the sides of the screen
        screen_x = self.px() - game.camera[0]
        if screen_x < SCREEN_LEFT:
            self.x = set_pixel(self.x, game.camera[0] + SCREEN_LEFT)
            self.vx = 0
        elif screen_x > SCREEN_RIGHT:
            self.x = set_pixel(self.x, game.camera[0] + SCREEN_RIGHT)
            self.vx = 0

        # 2. what the keys and the slope add to the speed sideways
        direction = -1 if self.left else (1 if self.right else 0)
        acceleration = self.input_term(direction)
        if self.state in (WALKING, SKIDDING, ROLLING):
            acceleration += self.slope
        if direction != 0 and self.state != SKIDDING and self.state != HURT:
            self.facing = direction

        # 3. the move sideways
        speed = self.vx + acceleration
        if (speed > 0 and self.wall_right) or (speed < 0 and self.wall_left):
            self.vx = 0
        else:
            self.vx = max(-self.cap, min(self.cap, speed))
            self.x += self.vx

        # 4. the move up or down
        if self.state in AIR:
            if self.state == JUMPING and self.jump_boost:
                if self.jump_held:
                    self.jump_counter += 1
                    if self.jump_counter < JUMP_HOLD_UPDATES:
                        self.vy = -JUMP_SPEED
                    else:
                        self.jump_boost = False
                else:
                    self.jump_boost = False
            self.vy += RAMP_GRAVITY if self.state == RAMP_JUMPING else GRAVITY
            self.vy = min(self.vy, MAX_FALL_SPEED)
        else:
            self.vy = GROUND_STICK
        self.y += self.vy

        # 5. the floor, at the one point under the middle of the feet
        fx, fy = self.px(), self.py()
        xb, yb = fx // TILE, fy // TILE
        column = fx - xb * TILE
        kind = level.block_type(xb, yb)
        if kind == EMPTY:
            self.on_floor = False
        elif kind == SOLID:
            height = level.ground_height(xb, yb, column)
            slope = level.slope_push(xb, yb)
            if height == TILE and level.block_type(xb, yb - 1) == SOLID:
                above = level.ground_height(xb, yb - 1, column)
                if above > 0:
                    height += above
                    slope = level.slope_push(xb, yb - 1)
            surface = (yb + 1) * TILE - height
            if fy >= surface:
                self.y = set_pixel(self.y, surface)
                self.on_floor = True
                self.slope = slope
        else:  # ONE_WAY
            surface = (yb + 1) * TILE - level.ground_height(xb, yb, column)
            if self.vy > 0 and fy >= surface and fy - surface < pixel(self.vy) + ONE_WAY_REACH:
                self.y = set_pixel(self.y, surface)
                self.on_floor = True
                self.slope = 0

        # 6. the walls: left, then right
        wall_y = self.py() - WALL_PROBE_Y
        probe = self.px() - WALL_PROBE_X
        self.wall_left = level.is_solid(probe, wall_y)
        if self.wall_left:
            out = probe
            for _ in range(TILE):
                out += 1
                if not level.is_solid(out, wall_y):
                    break
            self.x += (out - probe) * SUB
        probe = self.px() + WALL_PROBE_X
        self.wall_right = level.is_solid(probe, wall_y)
        if self.wall_right:
            out = probe
            for _ in range(TILE):
                out -= 1
                if not level.is_solid(out, wall_y):
                    break
            self.x -= (probe - out) * SUB

        # 7. the ceiling, when not on the floor
        if not self.on_floor:
            cy = self.py() - CEILING_PROBE_Y
            if level.is_solid(self.px(), cy):
                self.y = set_pixel(self.y, (cy // TILE + 1) * TILE + CEILING_PROBE_Y)
                self.vy = CEILING_BUMP_SPEED

        # 10. the jump
        if jump_pressed and self.state not in AIR and self.on_floor:
            self.state = JUMPING
            self.vy = -JUMP_SPEED
            self.on_floor = False
            self.jump_counter, self.jump_boost = 0, True
            return

        self.change_state(direction)

    def change_state(self, direction):
        state = self.state
        if state in AIR:
            if self.on_floor:
                self.state = WALKING
                self.cap = TOP_SPEED
            return
        if not self.on_floor:
            if state == ROLLING:
                if self.slope != 0 and self.vx * self.slope < 0:
                    self.state = RAMP_JUMPING
                    self.vy = -(abs(self.vx) + abs(self.vx) // 2)
                else:
                    self.state = JUMPING
                    self.vy = 0
                # the hold boost belongs to a jump that was pressed (the reference forgets to reset it: not copied)
                self.jump_boost = False
            else:
                self.state = FALLING
                self.vy = WALK_OFF_SPEED
            self.slope = 0
            return
        speed = abs(self.vx)
        moving = (self.vx > 0) - (self.vx < 0)
        if state == STANDING:
            if direction != 0:
                self.state = WALKING
            elif self.down:
                self.state = CROUCHING
            elif self.up:
                self.state = LOOKING_UP
        elif state == CROUCHING:
            if not self.down:
                self.state = STANDING
        elif state == LOOKING_UP:
            if direction != 0:
                self.state = WALKING
            elif not self.up:
                self.state = STANDING
        elif state == WALKING:
            if direction != 0 and direction == -moving and speed >= MIN_SKID_SPEED:
                self.state = SKIDDING
                self.skid_dir = moving
                self.facing = moving
            elif self.down and speed >= MIN_ROLL_SPEED:
                self.state = ROLLING
                self.cap = ROLL_TOP_SPEED
            elif direction == 0 and speed < STOP_SPEED:
                self.state = STANDING
                self.vx = 0
        elif state == SKIDDING:
            if moving == -self.skid_dir:
                self.state = WALKING
                self.cap = TOP_SPEED
            elif direction == 0 and speed < STOP_SPEED:
                self.state = STANDING
                self.vx = 0
                self.cap = TOP_SPEED
            elif direction != -self.skid_dir:
                # let go of the brake: back to walking (the reference only does this for one of the two sides)
                self.state = WALKING
                self.cap = TOP_SPEED
        elif state == ROLLING:
            if direction != 0 and direction == -moving:
                self.state = SKIDDING
                self.skid_dir = moving
                self.facing = moving
            elif self.slope == 0 and speed < ROLL_STOP_SPEED:
                self.state = CROUCHING if self.down else STANDING
                self.vx = 0
                self.cap = TOP_SPEED

    def take_damage(self, to_the_right):
        if self.immortal > 0 or self.state == HURT:
            return
        level = self.game.level
        self.immortal = HURT_UPDATES
        self.state = HURT
        self.vx = HURT_SPEED_X if to_the_right else -HURT_SPEED_X
        head = level.is_solid(self.px(), self.py() - CEILING_PROBE_Y - 1)
        self.vy = 0 if head else -HURT_SPEED_Y
        self.on_floor = False
        self.slope = 0
        self.jump_boost = False
        self.game.rings = 0

    def box(self):
        height = BALL_HEIGHT if self.is_ball() else BOX_HEIGHT
        return (self.px() - BOX_HALF_WIDTH, self.py() - height, 2 * BOX_HALF_WIDTH, height)


def intersects(a, b):
    return a[0] < b[0] + b[2] and b[0] < a[0] + a[2] and a[1] < b[1] + b[3] and b[1] < a[1] + a[3]


class Game:
    def __init__(self, tiles_text, level_text):
        self.level = Level(tiles_text, level_text)
        self.level_w, self.level_h = self.level.width * TILE, len(self.level.rows) * TILE
        self.player = Player(self)
        self.player.x = (self.level.start[0] + 16) * SUB
        self.player.y = (self.level.start[1] + 32) * SUB
        self.pickups = list(self.level.pickups)
        self.rings = 0
        self.updates = 0
        self.max_x, self.max_y = max(0, self.level_w - RES_W), max(0, self.level_h - RES_H)
        self.line = CAM_LINE_START
        self.camera = [max(0, min(self.player.px() - CAM_CENTRE_X, self.max_x)), max(0, min(self.player.py() - self.line, self.max_y))]

    def move_camera(self):
        p = self.player
        # the line the feet are kept near, one pixel an update towards where the state wants it
        target = CAM_LINE_LOOK_UP if p.state == LOOKING_UP else CAM_LINE_CROUCH if p.state == CROUCHING else CAM_LINE
        self.line += (target > self.line) - (target < self.line)
        sx = p.px() - self.camera[0]
        if sx > CAM_CENTRE_X + CAM_WINDOW_X:
            step = min(sx - (CAM_CENTRE_X + CAM_WINDOW_X), CAM_MAX_STEP)
            if self.camera[0] + step <= self.max_x:
                self.camera[0] += step
        elif sx < CAM_CENTRE_X - CAM_WINDOW_X:
            step = (CAM_CENTRE_X - CAM_WINDOW_X) - sx
            if step > CAM_QUIRK_STEP:
                step = CAM_MAX_STEP
            if self.camera[0] - step >= 0:
                self.camera[0] -= step
        sy = p.py() - self.camera[1]
        if sy > self.line + CAM_WINDOW_Y:
            step = min(sy - (self.line + CAM_WINDOW_Y), CAM_MAX_STEP)
            if self.camera[1] + step <= self.max_y:
                self.camera[1] += step
        elif sy < self.line - CAM_WINDOW_Y:
            step = (self.line - CAM_WINDOW_Y) - sy
            if step > CAM_QUIRK_STEP:
                step = CAM_MAX_STEP
            if self.camera[1] - step >= 0:
                self.camera[1] -= step

    def frame(self, left=False, right=False, jump=False, up=False, down=False, hurt=False):
        p = self.player
        p.left, p.right, p.up, p.down, p.jump_held = left, right, up, down, jump
        if hurt:
            p.take_damage(p.facing == -1)
        self.updates += 1
        self.move_camera()
        p.update()
        if p.state != HURT:
            box = p.box()
            for i in range(len(self.pickups) - 1, -1, -1):
                if intersects((self.pickups[i][0], self.pickups[i][1], 16, 16), box):
                    self.rings += 1
                    del self.pickups[i]

    def clock(self):
        seconds = self.updates // CLOCK_UPDATES_PER_SECOND
        return f"{seconds // 60}:{seconds % 60:02d}"


TILES = """G ground solid
S soil solid
a rise_low slope 31 16 1:2
b rise_high slope 15 0 1:2
c fall_high slope 0 15 1:2
d fall_low slope 16 31 1:2
r rise_steep slope 31 0 1:1
f fall_steep slope 0 31 1:1
1 rise_q1 slope 31 24 1:4
2 rise_q2 slope 23 16 1:4
3 rise_q3 slope 15 8 1:4
4 rise_q4 slope 7 0 1:4
5 fall_q1 slope 0 7 1:4
6 fall_q2 slope 8 15 1:4
7 fall_q3 slope 16 23 1:4
8 fall_q4 slope 24 31 1:4
= plank oneway
"""


def flat(width=64, rows=8, start=3):
    lines = ["." * width for _ in range(rows - 3)]
    lines.append("." * start + "P" + "." * (width - start - 1))
    lines += ["G" * width] * 2
    return "\n".join(lines)


def figures():
    """The derived figures of the reference, sections 4, 5 and 7, as a check that the model follows its rules."""
    g = Game(TILES, flat(96)); p = g.player
    for _ in range(10): g.frame()
    print("at rest:", NAMES[p.state], "feet", p.py(), "frame top", p.py() - 32)
    # rest to top speed
    x0, n = p.x, 0
    g.frame(right=True)  # standing: the state changes, the acceleration starts at the next update
    while p.vx < TOP_SPEED:
        g.frame(right=True); n += 1
    print(f"rest to 4.0: {n} updates, {(p.x - x0) / SUB:.2f} px      (reference: 60, 129)")
    for _ in range(20): g.frame(right=True)
    x0, n = p.x, 0
    while p.state != STANDING:
        g.frame(); n += 1
    print(f"friction stop from 4.0: {n} updates, {(p.x - x0) / SUB:.2f} px      (reference: 48, 100)")
    for _ in range(100): g.frame(right=True)
    x0, n = p.x, 0
    while p.vx >= 0:
        g.frame(left=True); n += 1
    print(f"skid from 4.0 to reversal: {n} updates, {(p.x - x0) / SUB:.2f} px      (reference: 31, 62)")

    for held, name, ref in ((1, "tap", "46.1 px, apex 22, air 45"), (40, "full", "94.8 px, apex 34, air 66")):
        g = Game(TILES, flat(32)); p = g.player
        for _ in range(10): g.frame()
        y0, top, apex, n = p.y, p.y, 0, 0
        g.frame(jump=True); held_left = held - 1
        while True:
            g.frame(jump=held_left > 0); held_left -= 1; n += 1
            if p.y < top:
                top, apex = p.y, n
            if p.on_floor:
                break
        print(f"jump {name}: {(y0 - top) / SUB:.2f} px, apex at update {apex}, in the air {n} updates      (reference: {ref})")

    g = Game(TILES, flat(32, start=10)); p = g.player
    for _ in range(10): g.frame()
    x0, y0, top, n = p.x, p.y, p.y, 0
    g.frame(hurt=True)
    while True:
        g.frame(); n += 1
        top = min(top, p.y)
        if p.on_floor:
            break
    print(f"hurt: {n} updates, {abs(p.x - x0) / SUB:.2f} px back, {(y0 - top) / SUB:.2f} px high      (reference: about 42, 42, 41)")

    # a roll from 6.0: down four tiles of 45 degrees, then a long flat
    rows = ["." * 80 for _ in range(12)]
    grid = [list(r) for r in rows]
    for c in range(80):
        top_row = 4 if c < 12 else (4 + (c - 11) if c < 16 else 9)
        if 12 <= c < 16:
            grid[4 + (c - 12)][c] = "f"
            for r in range(5 + (c - 12), 12): grid[r][c] = "G"
        else:
            for r in range(top_row if c < 12 else 8, 12): grid[r][c] = "G"
    grid[3][3] = "P"
    g = Game(TILES, "\n".join("".join(r) for r in grid)); p = g.player
    for _ in range(10): g.frame()
    while p.vx < TOP_SPEED: g.frame(right=True)
    while p.state != ROLLING: g.frame(right=True, down=True)
    fastest = 0
    while p.slope != 0 or p.px() < 16 * TILE:
        g.frame(); fastest = max(fastest, p.vx)
        if p.state != ROLLING: break
    x0, v0, n = p.x, p.vx, 0
    while p.state == ROLLING:
        g.frame(); n += 1
    print(f"roll: fastest {fastest} on the way down, {v0} on reaching the flat, then {n} updates and {(p.x - x0) / SUB:.2f} px to rest      (reference from 6.0: 294, 915)")


def run_level(args):
    repo = Path(args[0])
    frames, holds, trace = 0, [], 0
    i = 1
    while i < len(args):
        if args[i] == "--frames": frames = int(args[i + 1])
        elif args[i] == "--hold":
            keys, _, count = args[i + 1].partition(":"); holds.append((set(keys.split("+")), int(count) if count else 0))
        elif args[i] == "--trace": trace = int(args[i + 1])
        i += 2
    g = Game((repo / "assets" / "tiles.txt").read_text(), (repo / "assets" / "levels" / "test.txt").read_text())
    p = g.player
    seen = []
    for n in range(frames):
        keys, end = set(), 0
        for hold_keys, count in holds:
            if count == 0:
                keys = hold_keys; break
            end += count
            if n < end:
                keys = hold_keys; break
        g.frame("left" in keys, "right" in keys, "jump" in keys, "up" in keys, "down" in keys)
        if not seen or seen[-1][1] != p.state:
            seen.append((n, p.state))
        if trace and n % trace == 0:
            print(f"  {n:4d} x={p.x / SUB:8.2f} y={p.y / SUB:7.2f} vx={p.vx:5d} vy={p.vy:5d} {NAMES[p.state]:10s} cam={g.camera} gems={g.rings}")
    print(f"After {frames} frames at 50 Hz (clock {g.clock()}): player at {p.px() - 16},{p.py() - 32} level {g.level_w}x{g.level_h} camera {g.camera[0]},{g.camera[1]} picked up {g.rings}")
    print("   states:", ", ".join(f"{n}:{NAMES[s]}" for n, s in seen[:40]))


if __name__ == "__main__":
    if sys.argv[1] == "figures":
        figures()
    else:
        run_level(sys.argv[2:])
