"""tests/physics_tests.cpp, line for line, run on the Python model: the same levels, the same keys, the same
numbers expected. If this passes, the C++ tests expect what the model gives.

    python model_cpp_tests.py
"""
from model_ref import *  # noqa: F401,F403
import model_ref

checks = failures = 0
current = ""
game = player = None

_frame, _count = Game.frame, [0]


def _limited(self, *args, **keys):
    _count[0] += 1
    if _count[0] > 6000:
        raise RuntimeError("more than 6000 updates in one test: " + current)
    return _frame(self, *args, **keys)


Game.frame = _limited


def check(ok, what):
    global checks, failures
    checks += 1
    if not ok:
        failures += 1
        print(f"FAILED  {current}: {what}")


def equal(value, expected, what):
    check(value == expected, f"{what} (is {value}, expected {expected})")


def state(expected, what):
    check(player.state == expected, f"{what} (is {NAMES[player.state]})")


def dots(n):
    return "." * n


def start(name, rows):
    global current, game, player
    current = name
    _count[0] = 0
    game = Game(TILES, "\n".join(rows))
    player = game.player


def update(keys=""):
    game.frame(left="left" in keys, right="right" in keys, up="up" in keys, down="down" in keys, jump="jump" in keys, hurt="hurt" in keys)


def hold(keys, n):
    for _ in range(n):
        update(keys)


def wait(n):
    hold("", n)


def flat(width, start_tile=3):
    return [dots(width)] * 5 + [dots(start_tile) + "P" + dots(width - start_tile - 1)] + ["G" * width] * 2


def feet_x(): return player.px()
def feet_y(): return player.py()
def grounded(): return player.on_floor


def stands():
    start("stands where the level puts him", flat(32))
    wait(10)
    state(STANDING, "he stands")
    equal(feet_x(), 112, "x"); equal(feet_y(), 192, "y"); equal(feet_x() - 16, 96, "frame x"); equal(feet_y() - 32, 160, "frame y")
    equal(player.vx, 0, "speed"); equal(game.camera[0], 0, "camera x"); equal(game.camera[1], 64, "camera y"); equal(game.line, 128, "line")


def walks():
    start("walks up to top speed", flat(64))
    wait(10)
    for expected in (0, 32, 64, 96, 128, 144, 160):
        update("right"); equal(player.vx, expected, "first speeds")
    state(WALKING, "walking")
    start("walks up to top speed", flat(64))
    wait(10)
    update("right")
    origin, updates = player.x, 0
    while player.vx < TOP_SPEED and updates < 1000:
        update("right"); updates += 1
    equal(updates, 60, "updates to top speed"); equal(player.x - origin, 33024, "way")
    hold("right", 20); equal(player.vx, 1024, "stays")
    origin = player.x
    update(); equal(player.vx, 1004, "friction")
    updates = 1
    while player.state != STANDING and updates < 1000:
        update(); updates += 1
    equal(updates, 48, "updates to stop"); equal(player.x - origin, 25512, "way"); equal(player.vx, 0, "stopped")
    origin = player.x; wait(30); equal(player.x - origin, 0, "stays")


def skids():
    start("skids", flat(96))
    wait(10); hold("right", 100)
    origin = player.x
    update("left"); equal(player.vx, 1008, "first"); state(SKIDDING, "skids"); equal(player.facing, 1, "facing")
    update("left"); equal(player.vx, 976, "second")
    updates = 2
    while player.vx >= 0 and updates < 1000:
        update("left"); updates += 1
    equal(updates, 31, "updates to turn"); equal(player.x - origin, 16272, "way"); equal(player.vx, -16, "speed"); state(WALKING, "walking")
    update("left"); equal(player.facing, -1, "facing"); equal(player.vx, -48, "gaining")
    start("skids: letting go of the brake", flat(96))
    wait(10); hold("right", 100); hold("left", 5)
    equal(player.vx, 880, "five in"); state(SKIDDING, "skidding")
    update(); equal(player.vx, 860, "friction"); state(WALKING, "walks on")
    start("skids: not when slow", flat(96))
    wait(10); hold("right", 12); equal(player.vx, 240, "slow")
    update("left"); equal(player.vx, 224, "plain push"); state(WALKING, "no skid")


def jump(held, other=""):
    origin = player.y
    highest, top, landed, take_offs = origin, 0, 0, 0
    was = grounded()
    for i in range(200):
        update(other + "+jump" if i < held else other)
        if was and not grounded() and player.state == JUMPING: take_offs += 1
        was = grounded()
        if player.y < highest: highest, top = player.y, i
        if grounded() and i > 0 and landed == 0: landed = i
    return origin - highest, top, landed, take_offs


def jumps():
    start("jumps: the key going down", flat(32))
    wait(10); update("jump")
    state(JUMPING, "in a jump"); equal(player.vy, -1088, "speed"); equal(feet_y(), 192, "not moved yet")
    for held, rise, top, landed in ((1, 11792, 22, 45), (2, 11792, 22, 45), (3, 12832, 23, 47), (14, 24272, 34, 66), (200, 24272, 34, 66)):
        start(f"jumps: key held for {held} updates", flat(32))
        wait(10)
        result = jump(held)
        equal(result[0], rise, "rise"); equal(result[1], top, "top"); equal(result[2], landed, "landed"); equal(result[3], 1, "take offs")
        state(STANDING, "standing"); equal(feet_y(), 192, "ground")
    start("jumps: while running", flat(64))
    wait(10); hold("right", 100); update("right+jump"); equal(player.vx, 1024, "speed kept")
    updates = 0
    while not grounded() and updates < 1000:
        update("right+jump"); updates += 1
    equal(updates, 66, "air"); equal(player.vx, 1024, "speed"); state(WALKING, "walking")
    start("jumps: with no direction held", flat(64))
    wait(10); hold("right", 100); update("jump")
    speeds = []
    while not grounded() and len(speeds) < 1000:
        update("jump"); speeds.append(player.vx)
    equal(speeds[0], 988, "drag"); equal(speeds[1], 972, "drag"); equal(speeds[-1], 0, "none left"); equal(len(speeds), 66, "air")
    update(); state(STANDING, "stands")
    start("jumps: turning back in the air", flat(64))
    wait(10); hold("right", 100); update("jump")
    for expected in (940, 876, 812, 748, 684, 620, 556, 492, 428, 364, 300, 236, 172, 108, -20, -52, -84, -116, -148, -164):
        update("left+jump"); equal(player.vx, expected, "air braking")


def ledge():
    return [dots(32)] * 2 + ["...P" + dots(28)] + ["GGGGGGGG" + dots(24)] * 3 + ["G" * 32] * 2


def walks_off():
    start("walks off a ledge", ledge())
    wait(10); equal(feet_y(), 96, "on the ledge")
    left = landed = -1
    for i in range(120):
        update("right")
        if player.state == FALLING and left < 0:
            left = i
            equal(feet_x(), 257, "x"); equal(feet_y(), 103, "y"); equal(player.vy, 256, "vy")
        elif left >= 0 and i == left + 1:
            equal(player.vy, 304, "gravity")
        if left >= 0 and landed < 0 and grounded(): landed = i
    equal(left, 64, "left at"); equal(landed, 90, "landed at"); equal(feet_y(), 192, "ground"); equal(player.vx, 1024, "speed"); state(WALKING, "walking")


def way_down():
    rows = [list(dots(96)) for _ in range(12)]
    for c in range(96):
        top = 4 if c < 12 else (8 if c >= 16 else 4 + (c - 12) + 1)
        if 12 <= c < 16: rows[4 + (c - 12)][c] = "f"
        for r in range(top, 12): rows[r][c] = "G"
    rows[3][3] = "P"
    return ["".join(r) for r in rows]


def ramp():
    return [dots(64)] * 4 + ["...P" + dots(16) + "r" + dots(43)] + ["G" * 64] * 3


def rolls():
    start("rolls", flat(96))
    wait(10); hold("right", 12); update("down"); state(WALKING, "too slow")
    hold("right", 5); equal(player.vx, 300, "fast enough")
    update("right+down"); state(ROLLING, "rolling"); equal(player.vx, 316, "speed"); equal(player.cap, 1536, "cap")
    update("down"); equal(player.vx, 311, "friction")
    update("right"); equal(player.vx, 307, "forward")
    update("left"); equal(player.vx, 291, "back"); state(SKIDDING, "skid")
    update("left"); equal(player.vx, 259, "skid brake"); equal(player.facing, 1, "facing")
    start("rolls: to a stop", flat(96))
    wait(10); hold("right", 100); hold("right+down", 2)
    origin, updates = player.x, 0
    while player.state == ROLLING and updates < 1000:
        update("down"); updates += 1
    equal(updates, 190, "updates"); equal(player.x - origin, 102745, "way"); state(CROUCHING, "crouching")
    update(); state(STANDING, "standing")
    start("rolls: down a slope", way_down())
    wait(10)
    while feet_x() < 300: update("right")
    while player.state != ROLLING: update("right+down")
    fastest, in_air, gains = 0, 0, set()
    while feet_x() < 16 * 32:
        push, before = player.slope, player.vx
        update()
        if push == 31 and player.vx < 1536: gains.add(player.vx - before)
        fastest = max(fastest, player.vx)
        in_air += player.state in AIR
    check(gains == {26}, f"gains {gains}"); equal(fastest, 1536, "fastest"); equal(in_air, 0, "in the air"); equal(player.vx, 1536, "at the flat")
    origin, updates = player.x, 0
    while player.state == ROLLING and updates < 1000:
        update(); updates += 1
    equal(updates, 294, "updates"); equal(player.x - origin, 234369, "way"); state(STANDING, "standing"); equal(player.cap, 1024, "cap")
    start("rolls: off a ledge", ledge())
    wait(10)
    while player.vx < 600: update("right")
    left = False
    for i in range(120):
        if left and grounded(): break
        update("down")
        if player.state == JUMPING and not left:
            left = True
            equal(feet_x(), 257, "x"); equal(feet_y(), 103, "y"); equal(player.vy, 0, "vy"); equal(player.vx, 308, "vx")
    check(left, "rolls off"); state(WALKING, "walking"); equal(player.cap, 1024, "cap")
    start("rolls: off the end of a ramp", ramp())
    wait(10)
    while feet_x() < 580: update("right")
    while player.state != ROLLING: update("right+down")
    before = 0
    for i in range(200):
        if player.state == RAMP_JUMPING: break
        before = player.vx; update()
    state(RAMP_JUMPING, "thrown"); equal(before, 589, "before"); equal(player.vx, 553, "vx"); equal(player.vy, -829, "vy"); equal(feet_x(), 674, "x")
    update(); equal(player.vy, -793, "ramp gravity")
    highest = player.y
    while not grounded():
        update(); highest = min(highest, player.y)
    equal(pixel(highest), 99, "top"); equal(feet_x(), 709, "lands x"); equal(feet_y(), 160, "lands y")
    start("walks over the same ramp", ramp())
    wait(10)
    states = []
    for i in range(200):
        update("right")
        if not states or states[-1] != player.state: states.append(player.state)
    check(states == [WALKING, FALLING, WALKING], f"states {states}"); equal(feet_x(), 792, "x"); equal(feet_y(), 160, "y")


def hill():
    return [dots(64)] * 4 + [dots(28) + "rGGGGf" + dots(30), dots(22) + "abGGGG" + "SSSSSS" + "cd" + dots(28),
                             "...P" + dots(12) + "1234" + "GG" + "S" * 16 + "5678" + dots(22)] + ["G" * 64] * 2


def slopes():
    start("slopes: over the hill", hill())
    wait(10)
    in_air = updates = 0
    slowest = {-16: 9999, -24: 9999, -31: 9999}
    wrong = False
    while feet_x() < 1500 and updates < 2000:
        update("right"); updates += 1
        in_air += player.state in AIR
        x, y = feet_x(), feet_y()
        if (640 <= x <= 703 and y != 192) or (768 <= x <= 895 and y != 160) or (928 <= x <= 1055 and y != 128) or (1152 <= x <= 1215 and y != 192) or (x >= 1344 and y != 224):
            wrong = True
        if player.slope in slowest: slowest[player.slope] = min(slowest[player.slope], player.vx)
    equal(updates, 379, "updates"); equal(in_air, 0, "in the air"); check(not wrong, "heights")
    equal(slowest[-16], 1024, "1:4"); equal(slowest[-24], 896, "1:2"); equal(slowest[-31], 919, "1:1"); equal(player.vx, 1024, "speed")
    start("slopes: down with no keys", hill())
    wait(10)
    while feet_x() < 30 * 32: update("right")
    on11, on12, updates = set(), set(), 0
    while player.state != STANDING and updates < 600:
        push, before = player.slope, player.vx
        update(); updates += 1
        if push == 31: on11.add(player.vx - before)
        if push == 24: on12.add(player.vx - before)
    check(on11 == {11}, f"1:1 {on11}"); check(on12 == {4}, f"1:2 {on12}"); equal(updates, 117, "updates"); equal(feet_x(), 1188, "x"); equal(feet_y(), 192, "y")
    start("slopes: standing on one", hill())
    wait(10)
    while feet_x() < 23 * 32: update("right")
    wait(100); state(STANDING, "stops"); equal(feet_x(), 789, "x")
    wait(100); equal(feet_x(), 789, "stays"); equal(player.vx, 0, "speed")
    steep = [dots(32)] * 2 + [dots(12) + "rG" + dots(18), dots(11) + "rSG" + dots(18), dots(10) + "rSSG" + dots(18), "...P" + dots(5) + "rSSSG" + dots(18)] + ["G" * 32] * 2
    start("slopes: up 45 degrees from rest", steep)
    wait(10)
    while feet_x() < 9 * 32 - 2: update("right")
    wait(60); hold("right", 2)
    fastest = updates = 0
    while feet_y() > 192 - 4 * 32 and updates < 2000:
        update("right"); updates += 1
        if player.slope != 0: fastest = max(fastest, player.vx)
    equal(fastest, 128, "fastest"); equal(updates, 247, "updates"); equal(feet_y(), 64, "top")


def walls():
    rows = [dots(32)] + [dots(14) + "G" + dots(17)] * 4 + ["...P" + dots(10) + "G" + dots(17)] + ["G" * 32] * 2
    start("walls: on his right", rows)
    wait(10)
    updates = 0
    while True:
        before = player.vx; update("right"); updates += 1
        if (before == 1024 and player.vx == 0) or updates >= 1000: break
    equal(updates, 112, "updates"); equal(feet_x(), 437, "x")
    moved = False
    for i in range(300):
        update("right"); moved = moved or feet_x() != 437
    check(not moved, "moved")
    inside = False
    for i in range(100):
        update("right+jump" if i < 30 else "right"); inside = inside or feet_x() + WALL_PROBE_X >= 448
    check(not inside, "inside")
    rows = [dots(32)] + [dots(4) + "G" + dots(27)] * 4 + [dots(4) + "G" + dots(9) + "P" + dots(17)] + ["G" * 32] * 2
    start("walls: on his left", rows)
    wait(10); hold("left", 300); equal(feet_x(), 170, "x")


def sides():
    start("the sides of the picture", flat(32))
    wait(10); hold("left", 200)
    equal(feet_x(), 16, "x"); equal(feet_x() - 16, 0, "frame"); equal(game.camera[0], 0, "camera")
    outside = False
    for i in range(50):
        update("left"); outside = outside or not (15 <= feet_x() <= 16)
    check(not outside, "outside")
    hold("right", 600)
    equal(game.level_w, 1024, "level"); equal(game.camera[0], 765, "camera"); equal(feet_x(), 1012, "x")


def ceilings():
    rows = [dots(32)] * 3 + ["..GGGG" + dots(26), dots(32), "...P" + dots(28)] + ["G" * 32] * 2
    for k, keys in enumerate(("jump", "")):
        start("ceilings: a held jump" if k == 0 else "ceilings: a tap", rows)
        wait(10); update("jump")
        highest = player.y
        for i in range(120):
            update(keys); highest = min(highest, player.y)
        equal(pixel(highest), 152, "highest"); state(STANDING, "down again"); equal(feet_y(), 192, "ground")
    start("ceilings: the bump", rows)
    wait(10); update("jump")
    bumped = False
    for i in range(30):
        if bumped: break
        before = player.vy; update()
        if before < 0 and player.vy == 256: bumped = True
    check(bumped, "bumped"); equal(feet_y(), 152, "under it")


def one_way():
    rows = [dots(32)] * 3 + [dots(2) + "====" + dots(26), dots(32), "...P" + dots(28)] + ["G" * 32] * 2
    start("one-way platforms: from below", rows)
    wait(10); hold("right", 60); equal(feet_x(), 237, "x"); equal(feet_y(), 192, "y")
    start("one-way platforms: jumping up through", rows)
    wait(10)
    highest = player.y
    for i in range(200):
        update("jump" if i < 30 else ""); highest = min(highest, player.y)
    state(STANDING, "standing"); equal(feet_y(), 96, "on it"); equal(pixel(highest), 96, "caught")
    states = []
    for i in range(200):
        update("right")
        if not states or states[-1] != player.state: states.append(player.state)
    check(states == [WALKING, FALLING, WALKING], f"states {states}"); equal(feet_x(), 797, "x"); equal(feet_y(), 192, "y")


def hurt():
    rows = [dots(32)] * 5 + [dots(6) + "ooo" + dots(3) + "P" + dots(2) + "ooo" + dots(14)] + ["G" * 32] * 2
    start("hurt", rows)
    wait(10); hold("right", 40); wait(60)
    equal(game.rings, 1, "rings")
    from_x, from_y = player.x, player.y
    update("hurt")
    state(HURT, "hurt"); equal(player.vx, -256, "vx"); equal(player.vy, -976, "vy"); equal(game.rings, 0, "rings"); check(player.immortal > 0, "immortal")
    updates, highest = 1, player.y
    while not grounded() and updates < 1000:
        update("right"); updates += 1; highest = min(highest, player.y)
    equal(updates, 42, "updates"); equal(player.x - from_x, -10752, "back"); equal(from_y - highest, 10416, "high"); state(WALKING, "walking"); equal(player.vx, -256, "speed")
    update("hurt"); check(player.state != HURT, "second hit")
    updates = 0
    while player.immortal > 0 and updates < 1000:
        update(); updates += 1
    equal(updates, 77, "left")
    update("hurt"); state(HURT, "again")
    start("hurt: looking left", rows)
    wait(10); hold("left", 2); wait(30)
    update("hurt"); equal(player.facing, -1, "facing"); equal(player.vx, 256, "thrown right")
    while not grounded(): update()
    equal(game.rings, 0, "rings"); equal(feet_x(), 441, "x")


def picks_up():
    rows = [dots(32)] * 4 + [dots(6) + "o" + dots(25), "...P" + dots(2) + "o.o" + dots(23)] + ["G" * 32] * 2
    start("picks up", rows)
    wait(10)
    while game.rings == 0 and feet_x() < 400: update("right")
    equal(feet_x(), 197, "x")
    hold("right", 60); equal(game.rings, 2, "rings")


def camera():
    rows = [dots(64)] * 13 + ["...P" + dots(60)] + ["G" * 64] * 6
    start("camera", rows)
    equal(game.camera[0], 0, "x"); equal(game.camera[1], 328, "y"); equal(game.line, 120, "line")
    wait(10); equal(game.line, 128, "line"); equal(game.camera[1], 328, "y")
    steps, nearest, furthest, last = set(), 9999, -9999, game.camera[0]
    for i in range(200):
        update("right")
        steps.add(game.camera[0] - last); last = game.camera[0]
        sx = feet_x() - game.camera[0]
        nearest, furthest = min(nearest, sx), max(furthest, sx)
    check(min(steps) == 0 and max(steps) == 4, f"steps {steps}"); equal(nearest, 112, "nearest"); equal(furthest, 140, "furthest"); equal(game.camera[0], 657, "x")
    wait(100); equal(game.camera[0], 760, "x"); equal(game.camera[1], 328, "y")
    first = -1
    for i in range(120):
        update("up")
        if first < 0 and game.camera[1] != 328: first = i
    state(LOOKING_UP, "looking up"); equal(game.line, 184, "line"); equal(first, 9, "first move"); equal(game.camera[1], 280, "y")
    wait(120); equal(game.line, 128, "line"); equal(game.camera[1], 304, "y")
    hold("down", 200); state(CROUCHING, "crouching"); equal(game.line, 48, "line"); equal(game.camera[1], 384, "y")
    start("camera: a jump", rows)
    wait(10)
    highest, too_fast, last = game.camera[1], False, game.camera[1]
    for i in range(100):
        update("jump" if i < 30 else "")
        step = game.camera[1] - last; last = game.camera[1]
        too_fast = too_fast or abs(step) > 8
        highest = min(highest, game.camera[1])
    equal(highest, 241, "highest"); check(not too_fast, "too fast"); equal(game.camera[1], 304, "y")


def clock():
    start("clock", flat(32))
    wait(599); equal(game.updates // 60, 9, "9"); wait(1); equal(game.updates // 60, 10, "10")


for test in (stands, walks, skids, jumps, walks_off, rolls, slopes, walls, sides, ceilings, one_way, hurt, picks_up, camera, clock):
    test()
print(f"{checks} checks, {failures} failed (model; the C++ adds the clock at 60 Hz and the Entity test)")
