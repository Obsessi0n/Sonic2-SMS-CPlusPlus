"""Writes assets/levels/test.txt: the test level, 96 tiles by 12, laid out so that each rule of the movement
can be tried. The design is ours. Run:  python build_level.py <repo>"""
import sys
from pathlib import Path

W, H = 96, 12
grid = [["."] * W for _ in range(H)]

# The ground: for each column, the tile on the surface and the row it is in. Soil fills what is under it.
surface = {}
for c in range(0, 14): surface[c] = ("G", 9)                      # a flat start
for i, t in enumerate("1234"): surface[14 + i] = (t, 8)           # one up for four along
for c in range(18, 24): surface[c] = ("G", 8)
for i, t in enumerate("ab"): surface[24 + i] = (t, 7)             # one up for two along
for c in range(26, 32): surface[c] = ("G", 7)
surface[32] = ("r", 6)                                            # one up for one along
for c in range(33, 39): surface[c] = ("G", 6)                     # the top of the hill
surface[39] = ("f", 6)                                            # and all the way down again in one long run
for i, t in enumerate("cd"): surface[40 + i] = (t, 7)
for i, t in enumerate("5678"): surface[42 + i] = (t, 8)
for c in range(46, 50): surface[c] = ("G", 9)
surface[50] = ("r", 8)                                            # a ramp with nothing after it: a launch for a roll
for c in range(51, W): surface[c] = ("G", 9)
for c, (tile, row) in surface.items():
    grid[row][c] = tile
    for r in range(row + 1, H):
        grid[r][c] = "S"

def put(row, first, text):
    for i, ch in enumerate(text):
        grid[row][first + i] = ch

put(8, 3, "P")
put(8, 6, "ooooo")        # on the flat, at the height of the body
put(7, 19, "oooo")        # on the three steps of the hill
put(6, 27, "oooo")
put(5, 34, "oooo")
put(8, 46, "ooo")         # at the foot of the long way down
put(7, 60, "====")        # a platform to jump up through, two tiles above the ground
put(6, 60, "oooo")
put(5, 66, "====")        # and a second, two tiles above the first
put(4, 66, "oooo")
put(6, 74, "TTTT")        # a stone ceiling two tiles above the ground: a jump under it is cut short
put(8, 82, "TT")          # a wall one tile high
put(8, 87, "ooooo")

text = "\n".join("".join(row) for row in grid) + "\n"
Path(sys.argv[1], "assets", "levels", "test.txt").write_text(text, newline="\n")
for n, row in enumerate(grid):
    print(f"{n:2d} {''.join(row)}")
