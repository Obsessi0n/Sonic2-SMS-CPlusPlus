"""Draws the game's placeholder art: every picture here is made by this script, from nothing.

    python tools/make_art.py

It writes the tiles, the player's sheet, what is picked up and the numbers of the display under assets/.
Change a colour or a shape here and run it again; or draw better pictures by hand over its output, keeping
each picture's size and the place of each frame in it.

The pictures are BMP of 16 colours, four bits to a pixel, as the consoles of the time kept theirs: SDL reads
BMP by itself, so the game needs no library for pictures. Colour 0 of each picture's palette is see-through.
"""

import struct
from pathlib import Path

from PIL import Image, ImageDraw

ASSETS = Path(__file__).resolve().parent.parent / "assets"
CLEAR = (0, 0, 0, 0)
# Colour 0 of every palette: never drawn, it stands for the pixels that are not there.
SEE_THROUGH = (255, 0, 255)


def save_bmp(image, path):
    """Writes a picture as a BMP of 16 colours: 4 bits to a pixel, colour 0 for what is see-through."""
    image = image.convert("RGBA")
    width, height = image.size
    pixels = image.load()
    palette = [SEE_THROUGH]
    index = {}
    rows = []
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                row.append(0)
                continue
            if a != 255:
                raise ValueError(f"{path.name}: the pixel at {x},{y} is partly see-through; here a pixel is there or not")
            if (r, g, b) not in index:
                if (r, g, b) == SEE_THROUGH or len(palette) == 16:
                    raise ValueError(f"{path.name}: more than 15 colours, or the colour kept for see-through")
                index[(r, g, b)] = len(palette)
                palette.append((r, g, b))
            row.append(index[(r, g, b)])
        rows.append(row)

    # Each row of a BMP takes a whole number of 4 bytes, and the rows are kept from the bottom up.
    stride = (width * 4 + 31) // 32 * 4
    data = bytearray()
    for row in reversed(rows):
        line = bytearray(stride)
        for x, value in enumerate(row):
            line[x // 2] |= value << 4 if x % 2 == 0 else value
        data += line
    palette += [(0, 0, 0)] * (16 - len(palette))

    offset = 14 + 40 + 16 * 4
    header = struct.pack("<2sIHHI", b"BM", offset + len(data), 0, 0, offset)
    info = struct.pack("<IiiHHIIiiII", 40, width, height, 1, 4, 0, len(data), 2835, 2835, 16, 0)
    colours = b"".join(struct.pack("<4B", b, g, r, 0) for r, g, b in palette)
    path.write_bytes(header + info + colours + bytes(data))

GRASS, GRASS_DARK = (88, 176, 72), (52, 128, 56)
SOIL, SOIL_DARK, SOIL_LIGHT = (140, 92, 58), (104, 66, 42), (166, 116, 78)
STONE, STONE_DARK, STONE_LIGHT = (124, 132, 146), (88, 94, 108), (160, 168, 182)
WOOD, WOOD_DARK, WOOD_LIGHT = (176, 124, 72), (124, 82, 44), (208, 160, 104)

SUIT, SUIT_DARK = (240, 138, 36), (188, 98, 20)
VISOR, GLINT = (38, 48, 66), (150, 220, 255)
BOOT, GLOVE = (58, 62, 78), (236, 236, 228)

GEM, GEM_LIGHT, GEM_DARK = (86, 206, 196), (190, 250, 240), (36, 132, 140)
INK, PAPER = (24, 28, 40), (250, 250, 244)


def sheet(width, height):
    image = Image.new("RGBA", (width, height), CLEAR)
    return image, ImageDraw.Draw(image)


def speckle(draw, box, colors, step=5):
    """A few fixed specks inside a box, so that soil is not one flat colour and is the same every run."""
    x0, y0, x1, y1 = box
    for y in range(y0, y1):
        for x in range(x0, x1):
            if (x * 7 + y * 13) % (step * step) == 0:
                draw.point((x, y), colors[(x + y) % len(colors)])


# ---- tiles ------------------------------------------------------------------------------------

def soil_tile():
    image, draw = sheet(32, 32)
    draw.rectangle((0, 0, 31, 31), SOIL)
    speckle(draw, (0, 0, 32, 32), (SOIL_DARK, SOIL_LIGHT))
    return image


def grass_tile():
    image = soil_tile()
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 31, 5), GRASS)
    draw.line((0, 6, 31, 6), GRASS_DARK)
    for x in range(1, 32, 6):
        draw.line((x, 6, x, 8), GRASS_DARK)
    return image


def stone_tile():
    image, draw = sheet(32, 32)
    draw.rectangle((0, 0, 31, 31), STONE)
    for y in (0, 16):
        draw.line((0, y, 31, y), STONE_LIGHT)
        draw.line((0, y + 15, 31, y + 15), STONE_DARK)
    for x, y in ((15, 0), (7, 16), (23, 16)):
        draw.line((x, y, x, y + 15), STONE_DARK)
    return image


def plank_tile():
    """A wooden platform: a plank along the top of the square on two short brackets, and air under it.

    It is the picture of the one-way tile: the player jumps up through it and lands on it.
    """
    image, draw = sheet(32, 32)
    draw.rectangle((0, 0, 31, 5), WOOD)
    draw.line((0, 0, 31, 0), WOOD_LIGHT)
    draw.line((0, 5, 31, 5), WOOD_DARK)
    for x in (10, 21):
        draw.line((x, 1, x, 4), WOOD_DARK)
    for x in (3, 26):
        draw.rectangle((x, 6, x + 2, 9), WOOD_DARK)
    return image


def slope_tile(left, right):
    """Soil under a line that runs from `left` pixels down at the left edge to `right` at the right one, grass along it."""
    image, draw = sheet(32, 32)
    for x in range(32):
        top = round(left + (right - left) * x / 31)
        draw.line((x, top, x, 31), SOIL)
    speckle(draw, (0, 0, 32, 32), (SOIL_DARK, SOIL_LIGHT))
    # The specks fell on the whole square: clear what is over the line again, then lay the grass along it.
    for x in range(32):
        top = round(left + (right - left) * x / 31)
        if top > 0:
            draw.line((x, 0, x, top - 1), CLEAR)
        draw.line((x, top, x, min(31, top + 4)), GRASS)
        draw.point((x, min(31, top + 5)), GRASS_DARK)
    return image


# ---- the player -------------------------------------------------------------------------------

def figure(draw, ox, oy, legs=0, lean=0, arms=0):
    """A small courier in an orange suit with a dark visor, facing right, standing on the bottom of a 32x32 frame.

    `legs` is the step of the run (0 standing, 1 to 4 the stride), `lean` tips the body, `arms` swings them.
    """
    bx = ox + 9 + lean
    # Legs and boots.
    stride = {0: (0, 0), 1: (-3, 3), 2: (-1, 1), 3: (3, -3), 4: (1, -1)}[legs]
    for k, shift in enumerate(stride):
        x = ox + 11 + k * 6 + shift
        draw.rectangle((x, oy + 23, x + 3, oy + 28), SUIT_DARK)
        draw.rectangle((x - 1, oy + 29, x + 5, oy + 31), BOOT)
    # Body.
    draw.rounded_rectangle((bx, oy + 12, bx + 13, oy + 24), 3, SUIT)
    draw.line((bx + 2, oy + 20, bx + 11, oy + 20), SUIT_DARK)
    # Arms and gloves.
    draw.rectangle((bx - 2, oy + 14 + arms, bx, oy + 19 + arms), SUIT_DARK)
    draw.rectangle((bx - 3, oy + 19 + arms, bx, oy + 21 + arms), GLOVE)
    draw.rectangle((bx + 13, oy + 14 - arms, bx + 15, oy + 19 - arms), SUIT_DARK)
    draw.rectangle((bx + 13, oy + 19 - arms, bx + 16, oy + 21 - arms), GLOVE)
    # Helmet, visor and the antenna on top.
    draw.rounded_rectangle((bx + 1, oy + 2, bx + 13, oy + 13), 4, SUIT)
    draw.rounded_rectangle((bx + 5, oy + 5, bx + 13, oy + 10), 2, VISOR)
    draw.point((bx + 11, oy + 6), GLINT)
    draw.point((bx + 12, oy + 7), GLINT)
    draw.line((bx + 4, oy + 1, bx + 4, oy - 1 + 1), SUIT_DARK)
    draw.point((bx + 4, oy), GLINT)


def ball(draw, ox, oy, turn):
    """The same courier tucked into a ball, a quarter turn further each frame."""
    draw.ellipse((ox + 5, oy + 9, ox + 26, oy + 30), SUIT)
    draw.ellipse((ox + 5, oy + 9, ox + 26, oy + 30), outline=SUIT_DARK)
    cx, cy = ox + 15.5, oy + 19.5
    spokes = [((0, -9), (0, -3)), ((9, 0), (3, 0)), ((0, 9), (0, 3)), ((-9, 0), (-3, 0))]
    (ax, ay), (bx, by) = spokes[turn % 4]
    draw.line((cx + ax, cy + ay, cx + bx, cy + by), VISOR, width=3)
    (ax, ay), (bx, by) = spokes[(turn + 2) % 4]
    draw.line((cx + ax, cy + ay, cx + bx, cy + by), GLOVE, width=2)


def player_sheet():
    """Six rows of 32x32 frames, as the game reads them: 0 standing, 1 running (5), 3 the ball (5), 5 skidding (2)."""
    image, draw = sheet(32 * 5, 32 * 6)
    figure(draw, 0, 0)
    for frame, legs in enumerate((1, 2, 3, 4, 2)):
        figure(draw, 32 * frame, 32, legs=legs, lean=1, arms=(-2, -1, 2, 1, 0)[frame])
    for frame in range(5):
        ball(draw, 32 * frame, 32 * 3, frame)
    for frame in range(2):
        figure(draw, 32 * frame, 32 * 5, legs=1, lean=-2, arms=2)
        # Dust at the heels.
        for k in range(3):
            draw.point((32 * frame + 24 + k * 2 + frame, 32 * 5 + 30 - k - frame), PAPER)
    return image


# ---- what is picked up, and the display --------------------------------------------------------

def gem(draw, ox, oy, half):
    """A gem seen turning: `half` is half its width in this frame."""
    cx = ox + 8
    draw.polygon(((cx, oy + 2), (cx + half, oy + 8), (cx, oy + 14), (cx - half, oy + 8)), GEM, GEM_DARK)
    if half > 2:
        draw.line((cx - half + 2, oy + 7, cx - 1, oy + 4), GEM_LIGHT)


def pickup_sheet():
    image, draw = sheet(16 * 4, 16)
    for frame, half in enumerate((6, 4, 1, 4)):
        gem(draw, 16 * frame, 0, half)
    return image


DIGITS = {
    "0": ("111", "101", "101", "101", "111"),
    "1": ("010", "110", "010", "010", "111"),
    "2": ("111", "001", "111", "100", "111"),
    "3": ("111", "001", "111", "001", "111"),
    "4": ("101", "101", "111", "001", "001"),
    "5": ("111", "100", "111", "001", "111"),
    "6": ("111", "100", "111", "101", "111"),
    "7": ("111", "001", "010", "010", "010"),
    "8": ("111", "101", "111", "101", "111"),
    "9": ("111", "101", "111", "001", "111"),
}


def edged(draw, boxes):
    """Light boxes with a dark edge one pixel wide all round them: the edge of them all first, then the boxes over it."""
    for x0, y0, x1, y1 in boxes:
        draw.rectangle((x0 - 1, y0 - 1, x1 + 1, y1 + 1), INK)
    for box in boxes:
        draw.rectangle(box, PAPER)


def digit(draw, ox, oy, rows):
    """A number two pixels to the dot with a dark edge, in the left half of its cell: the game draws them half a cell apart.

    It is 12 pixels high in a cell of 16, two clear above and two below: the game stacks its two lines of numbers
    with nothing between the cells, and they must not run into each other.
    """
    edged(draw, [(ox + 1 + dx * 2, oy + 3 + dy * 2, ox + 2 + dx * 2, oy + 4 + dy * 2)
                 for dy, row in enumerate(rows) for dx, dot in enumerate(row) if dot == "1"])


def ui_sheet():
    """Two rows of 16x16 cells. Row 0: the lives mark in two cells, then 0 to 9. Row 1: the gem, then the colon."""
    image, draw = sheet(16 * 12, 32)
    # Lives: the courier's helmet, then a small cross for "times". The cross keeps to the left half of its cell:
    # the game draws the number of lives over the right half.
    draw.rounded_rectangle((2, 2, 13, 13), 4, SUIT)
    draw.rounded_rectangle((6, 5, 13, 10), 2, VISOR)
    draw.point((11, 6), GLINT)
    edged(draw, [(16 + 1 + k, 6 + k, 16 + 1 + k, 6 + k) for k in range(5)] + [(16 + 5 - k, 6 + k, 16 + 5 - k, 6 + k) for k in range(5)])
    for n in range(10):
        digit(draw, 16 * (2 + n), 0, DIGITS[str(n)])
    gem(draw, 0, 16, 5)
    # The colon of the clock. The game draws this cell 9 pixels right of the minutes and 2 higher than the numbers,
    # with the seconds 7 pixels further on: the dots sit at its left edge to fall halfway between the two.
    edged(draw, [(16 + 1, 16 + y, 16 + 2, 16 + y + 1) for y in (7, 11)])
    return image


def main():
    tiles = {
        "grass": grass_tile(),
        "soil": soil_tile(),
        "stone": stone_tile(),
        # A gentle rise takes two tiles to climb one, and a gentle fall two to drop one.
        "rise_low": slope_tile(31, 16),
        "rise_high": slope_tile(15, 0),
        "fall_high": slope_tile(0, 15),
        "fall_low": slope_tile(16, 31),
        # A steep one climbs a tile in one tile.
        "rise_steep": slope_tile(31, 0),
        "fall_steep": slope_tile(0, 31),
        # A shallow one takes four tiles to climb one: four quarters up, four down.
        "rise_q1": slope_tile(31, 24),
        "rise_q2": slope_tile(23, 16),
        "rise_q3": slope_tile(15, 8),
        "rise_q4": slope_tile(7, 0),
        "fall_q1": slope_tile(0, 7),
        "fall_q2": slope_tile(8, 15),
        "fall_q3": slope_tile(16, 23),
        "fall_q4": slope_tile(24, 31),
        "plank": plank_tile(),
    }
    (ASSETS / "tiles").mkdir(parents=True, exist_ok=True)
    (ASSETS / "sprites").mkdir(parents=True, exist_ok=True)
    for name, image in tiles.items():
        save_bmp(image, ASSETS / "tiles" / f"{name}.bmp")
    save_bmp(player_sheet(), ASSETS / "sprites" / "player.bmp")
    save_bmp(pickup_sheet(), ASSETS / "sprites" / "pickup.bmp")
    save_bmp(ui_sheet(), ASSETS / "sprites" / "ui.bmp")
    print(f"{len(tiles)} tiles and 3 sheets written under {ASSETS}")


if __name__ == "__main__":
    main()
