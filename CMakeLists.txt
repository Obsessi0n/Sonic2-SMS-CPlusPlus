cmake_minimum_required(VERSION 3.21)
project(platformer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# SDL2 and SDL2_image are fetched and built with the game, as static libraries: nothing has to be installed
# first, and no DLL is kept in the repository. The first build takes a few minutes for that.
include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

# Pictures are PNG, which SDL2_image reads by itself; the other formats would each ask for a library of their own.
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.32.10
  GIT_SHALLOW TRUE)
# SDL_image keeps the libraries for the other formats as git submodules; none is used here (PNG is read by the
# decoder that is inside SDL_image itself), so none is fetched: they are big and slow to clone.
FetchContent_Declare(SDL2_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG release-2.8.12
  GIT_SHALLOW TRUE
  GIT_SUBMODULES "")
FetchContent_MakeAvailable(SDL2 SDL2_image)

# Everything in src/ but main.cpp is the game itself, built once as a library: the program is main.cpp on top of
# it, and the tests of the physics are tests/physics_tests.cpp on top of the same.
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM GAME_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_library(engine STATIC ${GAME_SOURCES})
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine PUBLIC SDL2::SDL2-static SDL2_image::SDL2_image-static)

add_executable(platformer ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(platformer PRIVATE SDL2::SDL2main engine)

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

# Left alone, the player stays standing where the level puts him: on the ground, at tile (3,8). The picture is 192
# pixels tall, so the camera, centred on him, is 176 down the level. 300 updates at 50 a second are 6 seconds.
scripted_run(stands_on_the_ground "at 50 Hz \\(6 s\\): player at 96,256 level 3072x384 camera 0,176 picked up 0"
  --frames 300)
# At 60 updates a second the same updates are the same game; only the clock has moved less.
scripted_run(the_same_game_at_60 "at 60 Hz \\(5 s\\): player at 96,256 level 3072x384 camera 0,176 picked up 0"
  --hz 60 --frames 300)
scripted_run(runs_the_same_way_at_60 "at 60 Hz .*player at 2664,256 .* picked up 18"
  --hz 60 --frames 1600 --hold right)
# Any other rate is refused before the game starts.
scripted_run(refuses_another_rate "expected 50 \\(PAL\\) or 60 \\(NTSC\\)"
  --hz 55 --frames 10)
# A jump with the key held: 55 updates after it starts he is past its top (109 pixels up) and some 60 pixels above
# the ground, on the way down; later he is back on it.
scripted_run(jumps "player at 96,19[0-9] "
  --frames 60 --hold none:5 --hold jump:40)
scripted_run(lands_after_a_jump "player at 96,256 "
  --frames 200 --hold none:5 --hold jump:40)
# Running right: up the two rises, down the first fall, picking up the 18 gems that are on the way.
scripted_run(climbs_and_picks_up "player at 1[67][0-9][0-9],224 .* picked up 18"
  --frames 700 --hold right)
# Further on a wall one tile high stops him, with his right side against it (the wall is at x = 2688).
scripted_run(stops_at_a_wall "player at 2664,256 .* picked up 18"
  --frames 1600 --hold right)
# The left side of the level stops him too, and the camera does not look past it.
scripted_run(stops_at_the_left_side "player at -8,256 level 3072x384 camera 0,"
  --frames 300 --hold left)
# There and back: the slopes are climbed and come down in both directions.
scripted_run(there_and_back "player at -8,256 .* picked up 18"
  --frames 2800 --hold right:1300 --hold left)
