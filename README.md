# Momentum platformer engine (working title)

A 2D platformer engine in C++ and SDL2, written from scratch: a character that gains and keeps speed, ground with slopes, things to pick up, and a level format simple enough to edit in a text editor. Every line of code, every picture and every level here is the project's own.

It is a learning project. The aim is an engine with the full feel of the fast platformers of the 8-bit consoles (speed that builds, slopes that matter, rolling, loops) and then a game of its own on top of it.

It keeps to what such a console was: a picture of 256 by 192 pixels, and 50 updates a second, as a PAL machine drew. Start it with `--hz 60` for the 60 of an NTSC machine. Everything in the game is counted in updates, so the two differ as the consoles did: at 50 the same game goes by a fifth more slowly.

## Building

You need CMake 3.21 or later and a C++17 compiler. SDL2 and SDL2_image are downloaded and built with the game, so there is nothing else to install; the first build takes a few minutes because of that.

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/platformer
```

With MinGW on Windows add `-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++` to the first line. Without Ninja leave `-G Ninja` out. On Linux SDL needs the development packages of the window system to build: `.github/workflows/build.yml` has the list for Ubuntu.

The build copies `assets/` beside the program every time, so a changed level or picture is picked up by building again.

## Playing

| Key | Does |
| --- | --- |
| Left, Right | run |
| Z | jump: the longer it is held, the higher |
| J | take a hit (for testing) |

## Running it from a script

The program can be run with nobody at the keys, to check it from a script or on a machine with no screen:

```bash
platformer --frames 300
platformer --frames 700 --hold right --screenshot shot.bmp
platformer --frames 200 --hold right:60 --hold right+jump:20 --hold none
```

- `--hz 50` or `--hz 60` sets how many updates there are to a second; 50 when it is left out. It works with nobody at the keys and with somebody.
- `--frames N` runs N updates as fast as they go, draws once, says where the player ended up and leaves.
- `--hold KEYS` holds keys during the run: `left`, `right`, `jump`, joined by `+`, or `none`. `--hold KEYS:N` holds them for N updates only; several `--hold` follow one another in the order given. A jump needs the key to go down: put a `--hold` without `jump` before one with it.
- `--screenshot FILE.bmp` saves the frame drawn at the end, at the size of the window.

It ends with a line such as `After 300 frames at 50 Hz (6 s): player at 96,256 level 3072x384 camera 0,176 picked up 0`: how long that was on the game's clock, where the player is, how big the level is, where the camera is, and how much he picked up, the last four in pixels. The clock and the animations count updates, not seconds, so the same run gives the same line and the same picture every time. Set the environment variable `SDL_VIDEODRIVER=dummy` to run with no screen.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

runs two kinds of check:

- `physics`, the program `physics_tests` built from `tests/physics_tests.cpp`. It starts the game's rules with no window, no pictures and no keyboard, on small levels written in the test itself, holds keys for a number of updates and checks positions and speeds: standing, falling, running up to top speed, friction, braking, the height of a jump against how long the key is held, walls, ceilings, slopes, ledges, pickups, the camera. What it expects is worked out from the constants, so tuning them does not break it unless it breaks what a test is about.
- ten scripted runs of the program itself on `assets/levels/test.txt` (see `CMakeLists.txt`), each compared with the line it should end with. These do depend on the level and on the constants: when either changes, run them, look at what changed, and put the new numbers in.

`.github/workflows/build.yml` builds the game and runs all of this on Linux and on Windows at every push, and keeps a screenshot of the run.

## How the player moves

Everything about the movement is a number in `src/PhysicsManager.h`, in pixels per update: change one, build, and feel the difference. The numbers are the same at 50 updates a second and at 60.

- **On the ground** the player has one speed, along the ground, whatever way the ground leans. Holding a direction adds `ACCELERATION` to it up to `TOP_SPEED`; holding the other direction takes `DECELERATION` off (he skids); holding nothing takes `FRICTION` off until he stops.
- **Slopes** pull downhill by `SLOPE_FACTOR` times the sine of their angle, at every update. Going up he is slower; going down he gains speed past his top speed, and keeps it for as long as the direction is held. A gentle slope pulls less than the friction, so he can stand on it; a slope of 45 degrees pulls more, and he slides down it.
- **In the air** `GRAVITY` adds to his speed downwards up to `MAX_FALL_SPEED`, and a direction held adds `AIR_ACCELERATION` sideways. A jump throws him straight up at `JUMP_SPEED` on top of the speed he had; letting go of the key while he rises faster than `JUMP_RELEASE_SPEED` cuts the rise down to that, which is what makes a short press a small jump. When he lands, the part of his speed that runs along the ground is the speed he lands with.
- **The floor** is felt by two sensors under his feet. Each reads how high the ground stands under it, inside the tile, and the higher of the two is where he stands. Ground up to `STEP_HEIGHT` pixels above or below his feet is still ground: that keeps him on a slope on the way down, and lets him step on to a low ledge. Without ground that near he is in the air.
- **Walls and ceilings** are felt by a sensor on each side of his body, `STEP_HEIGHT` above his feet, and two at the top of his head. He is put back outside what they find themselves in.

This holds for ground up to about 45 degrees. There are no loops yet, and no running on walls or ceilings.

## Making a level

A level is a text file, one character to a tile of 32 by 32 pixels: see `assets/levels/test.txt`. What each character is, is in `assets/tiles.txt`:

```text
G grass     solid
a rise_low  slope 31 16
```

The first line says `G` is the tile whose picture is `assets/tiles/grass.png`, and that it is solid. The second is a slope: its surface is 31 pixels down from the tile's top at its left edge and 16 at its right. In a level `.` is air, `P` is where the player starts and `o` is something to pick up; put an `o` in the row the player's body is in when he passes, not the row above his head.

The sides and the bottom row of a level are solid, whatever is drawn there; above it there is only air.

When a tile is loaded its shape becomes 32 numbers, the height of the ground in each of its columns of pixels, and an angle. The physics reads only those, so a new shape of ground (a curve, a step) needs a new way of filling them in `MapLoader::ShapeGround` and nothing else.

## The pictures

All of them are drawn by `tools/make_art.py` (Python with Pillow), as placeholders until there is real art:

```bash
python tools/make_art.py
```

## Where things are

```text
src/main.cpp            the loop: a fixed 50 or 60 updates a second, drawing as often as it can; and the scripted runs
src/Game.*              what there is in the game and the order it is updated and drawn in
src/Player.*            the player: his speeds, his sensors, what a hit does
src/PhysicsManager.*    the constants of the movement, and what the level is at a point: solid or not, and where the floor is
src/MapLoader.*         the level, read once into memory, the shape of its tiles, and its drawing
src/EntityManager.*     the things to pick up
src/InputManager.*      keys to commands, from the keyboard or from a script
src/TextureManager.*    pictures and frames of animation
src/UserInterface.*     the count of what was picked up, the clock and the lives
tests/physics_tests.cpp the tests of the physics
```

## What comes next

- Rolling.
- A jump at right angles to the ground instead of straight up, and the player's picture leaning with the slope.
- Walls and ceilings the character can run along: quarter pipes and loops.
- What was picked up scattering when the player is hit; enemies; springs.
- A camera that looks ahead and does not follow every jump.
