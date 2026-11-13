"""Draws the whole test level from assets/ as the game would (tiles and gems, no camera), in three strips."""
import sys
from pathlib import Path

from PIL import Image

repo, out = Path(sys.argv[1]), Path(sys.argv[2])


def picture(path):
    """A picture of the game: a BMP of 16 colours whose colour 0 is see-through, as the game reads it."""
    image = Image.open(path)
    image.info["transparency"] = 0
    return image.convert("RGBA")


kinds = {}
for line in (repo / "assets" / "tiles.txt").read_text().splitlines():
    words = line.split()
    if line and line[0] != "#" and len(words) >= 3:
        kinds[words[0]] = picture(repo / "assets" / "tiles" / f"{words[1]}.bmp")
rows = (repo / "assets" / "levels" / "test.txt").read_text().splitlines()
gem = picture(repo / "assets" / "sprites" / "pickup.bmp").crop((0, 0, 16, 16))
hero = picture(repo / "assets" / "sprites" / "player.bmp").crop((0, 0, 32, 32))
width = max(len(r) for r in rows) * 32
level = Image.new("RGBA", (width, len(rows) * 32), (120, 190, 235, 255))
for y, row in enumerate(rows):
    for x, ch in enumerate(row):
        if ch in kinds:
            level.alpha_composite(kinds[ch], (x * 32, y * 32))
        elif ch == "o":
            level.alpha_composite(gem, (x * 32 + 8, y * 32 + 8))
        elif ch == "P":
            level.alpha_composite(hero, (x * 32, y * 32))
third = width // 3
for n in range(3):
    level.crop((n * third, 64, (n + 1) * third, len(rows) * 32)).convert("RGB").save(out / f"level_strip_{n + 1}.png")
print("written", width, "x", len(rows) * 32)
