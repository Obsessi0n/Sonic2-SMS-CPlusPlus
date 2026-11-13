cmake_minimum_required(VERSION 3.21)
project(platformer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Built for size: the game's own work is small enough that code made for speed buys nothing a player would see.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

# The program as small as it can be made: every function and every piece of data in a section of its own, so
# that the link leaves out what nothing calls. Set before SDL is added, so that SDL is built the same way.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE)
  add_compile_options(-ffunction-sections -fdata-sections)
  add_link_options(-Wl,--gc-sections)
endif()

# SDL2 is fetched and built with the game, as a static library: nothing has to be installed first, and no DLL is
# kept in the repository. The first build takes a few minutes for that.
include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

# Only the parts of SDL the game uses: a window and its events, drawing, the clock, files. Sound, game
# controllers, haptics, sensors and the rest are left out of the program; when the game needs one, take it off
# this list.
foreach(part AUDIO JOYSTICK HAPTIC HIDAPI POWER SENSOR LOCALE MISC)
  set(SDL_${part} OFF CACHE BOOL "" FORCE)
endforeach()
# Drawing goes through Direct3D 9 or 11 on Windows, which every Windows has, and OpenGL or OpenGL ES on Linux; and
# through software where none of them starts. Vulkan is not drawn with, and the off-screen video driver is not
# needed: the tests run with the "dummy" one.
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_OFFSCREEN OFF CACHE BOOL "" FORCE)
if(WIN32)
  set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
  set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
  # SDL has no switch of its own for Direct3D 12: it is built when the header is found, and this is the answer
  # of that search, given before it is made.
  set(HAVE_D3D12_H 0 CACHE INTERNAL "")
endif()

FetchContent_Declare(SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.32.10
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(SDL2)

# Everything in src/ but main.cpp is the game itself, built once as a library: the program is main.cpp on top of
# it, and the tests of the physics are tests/physics_tests.cpp on top of the same.
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM GAME_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_library(engine STATIC ${GAME_SOURCES})
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine PUBLIC SDL2::SDL2-static)

add_executable(platformer ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(platformer PRIVATE SDL2::SDL2main engine)
# The names of the functions are of use only to a debugger: a build that is not for debugging leaves them out.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(platformer PRIVATE $<$<NOT:$<CONFIG:Debug>>:-s>)
endif()

# The tests open no window and have a plain main() of their own: SDL is told not to put its own in front of it.
add_executable(physics_tests ${CMAKE_SOURCE_DIR}/tests/physics_tests.cpp)
target_compile_definitions(physics_tests PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(physics_tests PRIVATE engine)

if(MINGW)
  # The compiler's own libraries inside the programs too, so they run on a machine that has no MinGW.
  target_link_options(platformer PRIVATE -static)
  target_link_options(physics_tests PRIVATE -static)
endif()

# The game reads its assets from the folder it is started in: keep a copy beside the program. It is copied at
# every build, not only when the program itself is built again: a level or a picture changes more often than the code.
add_custom_target(assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:platformer>/assets
  COMMENT "Copying assets beside the program")

# Run all the checks below with `ctest --test-dir build --output-on-failure`.
enable_testing()

# The tests of the physics: the game's rules with no window, on small levels of their own.
add_test(NAME physics COMMAND physics_tests)

# Then the program itself, with no screen and nobody at the keys: run for a number of updates with keys held by a
# script (see the top of src/main.cpp), and what it says at the end compared with what is expected on
# assets/levels/test.txt.

function(scripted_run name expected)
  add_test(NAME ${name}
    COMMAND platformer ${ARGN}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:platformer>)
  set_tests_properties(${name} PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy"
    PASS_REGULAR_EXPRESSION "${expected}"
    TIMEOUT 60)
endfunction()

# Everything in the movement is a whole number, so each run ends on exactly the line given here. "player at" is
# the top left corner of his 32 by 32 frame: the middle of his feet is 16 to the right of it and 32 below.

# Left alone, the player stays standing where the level puts him: on the ground, at tile (3,8). The camera keeps
# his feet 120 pixels down the picture. The level's clock counts 60 updates to its second: 300 are 5 seconds.
scripted_run(stands_on_the_ground "at 50 Hz \\(clock 0:05\\): player at 96,256 level 3072x384 camera 0,168 picked up 0"
  --frames 300)
# At 60 updates a second the same updates are the same game, and the same time on its clock.
scripted_run(the_same_game_at_60 "at 60 Hz \\(clock 0:05\\): player at 96,256 level 3072x384 camera 0,168 picked up 0"
  --hz 60 --frames 300)
scripted_run(runs_the_same_way_at_60 "at 60 Hz \\(clock 0:25\\): player at 2597,256 level 3072x384 camera 2477,144 picked up 20"
  --hz 60 --frames 1500 --hold right)
# Any other rate is refused before the game starts.
scripted_run(refuses_another_rate "expected 50 \\(PAL\\) or 60 \\(NTSC\\)"
  --hz 55 --frames 10)
# A jump with the key held is 95 pixels high, with its top 34 updates after it starts: here he is seen at the
# top (the jump starts at update 5), and then back on the ground.
scripted_run(jumps "player at 96,161 level 3072x384 camera 0,81 picked up 0"
  --frames 40 --hold none:5 --hold jump:40)
scripted_run(lands_after_a_jump "player at 96,256 level 3072x384 camera 0,144 picked up 0"
  --frames 200 --hold none:5 --hold jump:40)
# Walking right: up the three slopes of the hill (1:4, 1:2, 1:1), over its top and down again, picking up the
# 20 gems on the way, off the end of the ramp, and on to a wall one tile high. The wall's face is at x = 2624:
# the middle of his feet stops 11 pixels from it.
scripted_run(walks_over_the_hill_to_the_wall "player at 2597,256 level 3072x384 camera 2477,144 picked up 20"
  --frames 700 --hold right)
scripted_run(stays_at_the_wall "player at 2597,256 level 3072x384 camera 2477,144 picked up 20"
  --frames 1500 --hold right)
# The left end of the level stops him with his frame just inside it.
scripted_run(stops_at_the_left_end "player at 0,256 level 3072x384 camera 0,168 picked up 0"
  --frames 300 --hold left)
# Coming back, the ramp is a wall from behind: its face is at x = 1632, and on this side the middle of his feet
# stops 10 pixels from it.
scripted_run(the_back_of_the_ramp_is_a_wall "player at 1626,256 level 3072x384 camera 1522,144 picked up 20"
  --frames 3000 --hold right:1300 --hold left)
# Down on the top of the hill starts a roll. It gains speed all the way down, leaves the end of the ramp upwards
# and comes down 180 pixels further on, where it stops.
scripted_run(rolls_down_the_hill_and_off_the_ramp "player at 1784,256 level 3072x384 camera 1664,144 picked up 20"
  --frames 700 --hold right:290 --hold down:60 --hold none)
# A running jump up through the first wooden platform, and braking on top of it: its top is at y = 224.
scripted_run(lands_on_a_platform "player at 1941,192 level 3072x384 camera 1821,81 picked up 21"
  --frames 600 --hold right:455 --hold right+jump:30 --hold left:14 --hold none)
