# Momentum platformer engine (working title)

A 2D platformer engine in C++ and SDL2, written from scratch: a character that gains and keeps speed, ground with slopes, things to pick up, and a level format simple enough to edit in a text editor. Every line of code, every picture and every level here is the project's own.

It is a learning project. The aim is an engine with the full feel of the fast platformers of the 8-bit consoles (speed that builds, slopes that matter, rolling, loops) and then a game of its own on top of it.

It keeps to what such a console was: a picture of 256 by 192 pixels, and 50 updates a second, as a PAL machine drew. Start it with `--hz 60` for the 60 of an NTSC machine. Everything in the game is counted in updates, so the two differ as the consoles did: at 50 the same game goes by a fifth more slowly.

## Building

You need CMake 3.21 or later and a C++17 compiler. SDL2 is downloaded and built with the game, so there is nothing else to install; the first build takes a few minutes because of that.

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/platformer
```

With MinGW on Windows add `-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++` to the first line. Without Ninja leave `-G Ninja` out. On Linux SDL needs the development packages of the window system to build: `.github/workflows/build.yml` has the list for Ubuntu.

The build copies `assets/` beside the program every time, so a changed level or picture is picked up by building again.

## What it asks of the machine

As little as it can without leaving anything out of the game:

- **Disk.** The program is a single file of about 1.3 MB with all of SDL inside it, and the pictures, the tiles and the level take about 34 KB. It is built for size (`MinSizeRel`), with only the parts of SDL the game uses (the list is in `CMakeLists.txt`); the link leaves out whatever nothing calls, and the names only a debugger needs are stripped. The game does not use C++ streams, which alone would add most of a megabyte, nor a library for pictures.
- **Processor.** 50 updates a second (or 60), a frame drawn after each, and asleep in between: it does not spin a core waiting for the next update. Nothing is drawn while the window is minimised.
- **Memory.** What SDL and the graphics driver need, and a few kilobytes of pictures. The window is no bigger than the screen needs (see `--scale` below), and so neither is what is drawn.
- **Graphics.** Direct3D 9 or 11 on Windows, OpenGL or OpenGL ES on Linux; where none of them starts, it draws in software.

It needs a 64-bit Windows 10 or later, or a 64-bit Linux with X11 or Wayland, and a keyboard. To put back a part of SDL the game comes to need (sound, game controllers), take it off the list in `CMakeLists.txt`.

## Playing

| Key | Does |
| --- | --- |
| Left, Right | walk and run; against his own movement, skid |
| Down | roll, when he is moving; crouch, when he stands (the view moves down) |
| Up | look up, when he stands (the view moves up) |
| Z | jump: held for the first quarter of a second it goes twice as high |
| J | take a hit (for testing) |

## Running it from a script

The program can be run with nobody at the keys, to check it from a script or on a machine with no screen:

```bash
platformer --frames 300
platformer --frames 700 --hold right --screenshot shot.bmp
platformer --frames 200 --hold right:60 --hold right+jump:20 --hold none
platformer --frames 700 --hold right:290 --hold down:60 --hold none
```

- `--hz 50` or `--hz 60` sets how many updates there are to a second; 50 when it is left out. It works with nobody at the keys and with somebody.
- `--scale N` makes the window N times the picture (1 to 16); when it is left out, the biggest that fits on the screen, up to 4. It changes nothing in the game, only the size it is seen at.
- `--frames N` runs N updates as fast as they go, draws once, says where the player ended up and leaves.
- `--hold KEYS` holds keys during the run: `left`, `right`, `up`, `down`, `jump`, joined by `+`, or `none`. `--hold KEYS:N` holds them for N updates only; several `--hold` follow one another in the order given. A jump needs the key to go down: put a `--hold` without `jump` before one with it.
- `--screenshot FILE.bmp` saves the frame drawn at the end, at the size of the window.

It ends with a line such as `After 300 frames at 50 Hz (clock 0:05): player at 96,256 level 3072x384 camera 0,168 picked up 0`: what the level's clock shows, where the top left corner of the player's frame is, how big the level is, where the camera is, and how much he picked up, the last four in pixels. Everything in the game is a whole number counted in updates, so the same run gives exactly the same line and the same picture every time, on every machine. Set the environment variable `SDL_VIDEODRIVER=dummy` to run with no screen.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

runs two kinds of check:

- `physics`, the program `physics_tests` built from `tests/physics_tests.cpp`. It starts the game's rules with no window, no pictures and no keyboard, on small levels written in the test itself, holds keys for a number of updates and checks positions and speeds to the exact number: walking up to top speed, friction, skidding, the height of a jump against how long the key is held, the air, ledges, rolling, the three classes of slope, walls, ceilings, one-way platforms, being hit, pickups, the camera and the clock.
- twelve scripted runs of the program itself on `assets/levels/test.txt` (see `CMakeLists.txt`), each compared with the line it should end with.

Both expect exact numbers. A change to a constant or to the test level is a change to the game: run the tests, look at what changed, and put the new numbers in.

`.github/workflows/build.yml` builds the game and runs all of this on Linux and on Windows at every push, and keeps a screenshot of the run.

## How the player moves

The player moves by the rules of the 8-bit console platformers the engine is modelled on, as the project's movement reference sets them out. Everything is a whole number: a position is counted in 1/256 of a pixel, a speed in 1/256 of a pixel per update. There is a speed sideways and a speed downwards and nothing else: no angle, no speed "along the ground". Every number is in `src/PhysicsManager.h`, with the reference's raw value and the same in pixels beside it; the numbers are the same at 50 updates a second and at 60.

- **Walking.** A direction held adds 16 to his speed sideways every update, up to 4 pixels an update. With no direction the ground takes 20 off; slower than an eighth of a pixel he stops dead. Slower than half a pixel an update every one of these counts double. From rest it takes 60 updates and 129 pixels to reach top speed, and 48 updates and 100 pixels to stop.
- **Skidding.** The other direction, when he is going at a pixel an update or more, is a skid: it brakes twice as hard as walking pushes, until he has turned round.
- **Rolling.** Down, at a pixel an update or more, curls him into a ball. A roll cannot be pushed: it only loses speed, a quarter as fast as walking does, and on the flat it ends when it has all but stopped. Its top speed is 6 pixels an update, which only a way down can give it.
- **Slopes** push downhill by a fixed amount for their class, whatever his speed: 31 for one up for one along, 24 for 1:2, 16 for 1:4. He moves sideways by his speed and is then put on the surface, so along a steep slope he covers more ground. A slope does not push a player who is standing. Walking up 45 degrees from rest he cannot get above a crawl; rolling down it he gains 26 every update.
- **Jumping.** A jump is a speed upwards of 4.25 pixels an update and nothing else: his speed sideways stays. While the key stays down, for 13 updates, he keeps that speed; then gravity (48 every update) takes over. A tap rises 46 pixels, a held key 95. Letting go does not cut the rise he already has.
- **In the air** a direction with his movement adds 16, against it takes 64 off, and none at all loses 16 an update: a jump with no direction held comes straight down. Walking off a ledge is a fall, with less say over it; rolling off one keeps him a ball; rolling off the end of a ramp, uphill, throws him up at one and a half times his speed sideways.
- **The floor** is felt at one point, the middle of his feet: he falls as soon as his middle is past an edge. On the ground he is driven 7 pixels down every update and lifted back to the surface, which is all that keeps him on a slope going down. The surface is read from the height of the ground in that column of the tile, and of the tile above when the column is full.
- **Walls** are felt 10 pixels to each side of his middle, 12 above his feet: anything lower is a step he walks up. A wall stops him one update after he reaches it. **Ceilings** are felt 24 above his feet, in the air only, and send him back down. **One-way platforms** catch him only on the way down.
- **Being hit** throws him back at a pixel an update and up at four, with no say until he lands, takes what he picked up, and keeps him out of harm for 120 updates.
- **The sides of the picture** stop him: the camera stops at the ends of the level, and he cannot leave the picture.

His picture is a 32 by 32 frame; the point he is at is the middle of its bottom edge. `src/Player.h` says where the other points are from there.

Where the reference lists slips of the original (a skid that only cancels on one side, a drag that swings about zero, and three more), the code does the sensible thing and says in a comment what the slip was.

## The camera and the clock

The camera does not sit on the player. Sideways he may move 8 pixels either way from the middle of the picture before it follows; up and down his feet may be 16 pixels either way from a line 128 pixels down the picture. It follows by as much as he is outside, never more than 8 pixels an update, and it is always one update behind him. Looking up moves the line to 184 and crouching to 48, a pixel an update, which pans the view. It does not move past the ends of the level.

The clock of the level counts 60 updates to its second at either rate: at 50 updates a second a second on the clock lasts 1.2 real ones, as everything else does.

## Making a level

A level is a text file, one character to a tile of 32 by 32 pixels: see `assets/levels/test.txt`. What each character is, is in `assets/tiles.txt`:

```text
G grass     solid
a rise_low  slope 31 16 1:2
= plank     oneway
```

The first line says `G` is the tile whose picture is `assets/tiles/grass.bmp`, and that it is solid: a floor on top, a wall at its sides, a ceiling underneath. The second is a slope: its surface is 31 pixels down from the tile's top at its left edge and 16 at its right, and it belongs to a slope of one up for two along, which decides how hard it pushes downhill (`1:1`, `1:2` or `1:4`). The third is a platform that is only a floor, and only from above. In a level `.` is air, `P` is where the player starts and `o` is something to pick up; put an `o` in the row the player's body is in when he passes, not the row above his head.

The file has the slopes of all three classes, going up and going down: one tile for 1:1, two for 1:2, four for 1:4. A slope carries on from one tile into the tile above it, so put solid tiles under the higher part of a slope, as the test level does.

The bottom row of a level is solid, whatever is drawn there; above the level there is only air. Its two ends are where the picture stops.

When a tile is loaded its shape becomes 32 numbers, the height of the ground in each of its columns of pixels, and the push of its slope. The movement reads only those, so a new shape of ground (a curve, a step) needs a new way of filling them in `MapLoader::ShapeGround` and nothing else.

## The pictures

All of them are drawn by `tools/make_art.py` (Python with Pillow), as placeholders until there is real art:

```bash
python tools/make_art.py
```

They are BMP pictures of 16 colours, four bits to a pixel, the way the consoles of the time kept theirs, and colour 0 of each palette is see-through. SDL reads BMP by itself, so the game needs no library for pictures. A picture drawn by hand must keep to that: 15 colours and the see-through one, and no pixel partly see-through. `make_art.py` refuses a picture that does not.

## Where things are

```text
src/main.cpp            the loop: a fixed 50 or 60 updates a second, a frame drawn after each, asleep in between; and the scripted runs
src/Game.*              what there is in the game, the order it is updated and drawn in, the camera and the clock
src/Player.*            the player: what he is doing, his speeds, the points he feels the level with, what a hit does
src/PhysicsManager.*    every number of the movement, and what the level is at a point: solid or not, and where the floor is
src/MapLoader.*         the level, read once into memory, the shape and the class of its tiles, and its drawing
src/EntityManager.*     the things to pick up
src/InputManager.*      keys to commands, from the keyboard or from a script
src/TextureManager.*    pictures and frames of animation
src/UserInterface.*     the count of what was picked up, the clock and the lives
tests/physics_tests.cpp the tests of the physics
```

## What comes next

- Pictures for crouching, looking up and balancing on an edge: for now he is drawn standing.
- Springs, and the lighter gravity that goes with them.
- What was picked up scattering when the player is hit; enemies, and bouncing off them as a ball.
- Loops, which in this kind of engine are paths the player is carried along, and water.
- Sound, ticking 60 times a second at either rate.
