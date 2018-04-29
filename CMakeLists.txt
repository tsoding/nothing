cmake_minimum_required(VERSION 3.2)
project(nothing)

if(WIN32)
  # Conan integration
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(SDL2 REQUIRED)

find_package(SDL2_mixer REQUIRED)

include_directories(src/)
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_MIXER_INCLUDE_DIR})

set(SOURCE_FILES
  src/color.c
  src/game.c
  src/game/level.c
  src/game/level/background.c
  src/game/level/camera.c
  src/game/level/goals.c
  src/game/level/lava.c
  src/game/level/lava/wavy_rect.c
  src/game/level/platforms.c
  src/game/level/player.c
  src/game/level/player/dying_rect.c
  src/game/level/player/rigid_rect.c
  src/game/sound_medium.c
  src/main.c
  src/math/mat3x3.c
  src/math/point.c
  src/math/rand.c
  src/math/rect.c
  src/math/triangle.c
  src/sdl/renderer.c
  src/system/error.c
  src/system/lt.c
  src/system/lt/lt_slot.c
)

set(HEADER_FILES
  src/color.h
  src/game.h
  src/game/level.h
  src/game/level/background.h
  src/game/level/camera.h
  src/game/level/goals.h
  src/game/level/lava.h
  src/game/level/lava/wavy_rect.h
  src/game/level/platforms.h
  src/game/level/player.h
  src/game/level/player/dying_rect.h
  src/game/level/player/rigid_rect.h
  src/game/sound_medium.h
  src/math/mat3x3.h
  src/math/pi.h
  src/math/point.h
  src/math/rand.h
  src/math/rect.h
  src/math/triangle.h
  src/sdl/renderer.h
  src/system/error.h
  src/system/lt.h
  src/system/lt/lt_slot.h
)

add_executable(nothing ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(nothing ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "CLANG"))
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} \
     -Wall \
     -Werror \
     -Wextra \
     -Wconversion \
     -Wunused \
     -Wunused-function \
     -Wunused-label \
     -Wunused-macros \
     -Wunused-parameter \
     -Wunused-value \
     -Wunused-variable \
     -Wcast-align \
     -Wcast-qual \
     -Wmissing-declarations \
     -Wredundant-decls \
     -Wmissing-prototypes \
     -Wnested-externs \
     -Wpointer-arith \
     -Wshadow \
     -Wstrict-prototypes \
     -Wwrite-strings \
     -fno-common \
     -pedantic \
     -std=c11 \
     -O3")
  target_link_libraries(nothing m)
endif()
if(WIN32)
  target_link_libraries(nothing Imm32 Version winmm)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sounds DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
