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
  src/main.c
  src/game/level/player.c
  src/game/level/platforms.c
  src/rect.c
  src/point.c
  src/game/level/camera.c
  src/error.c
  src/game.c
  src/lt.c
  src/lt/lt_slot.c
  src/game/level.c
  src/game/level/goals.c
  src/renderer.c
  src/triangle.c
  src/color.c
  src/rigid_rect.c
  src/dying_rect.c
  src/game/level/lava.c
  src/wavy_rect.c
  src/game/sound_medium.c
  src/path.c
  src/mat3x3.c
  src/algo.c
)

set(HEADER_FILES
  src/game/level/player.h
  src/game/level/platforms.h
  src/rect.h
  src/point.h
  src/game/level/camera.h
  src/error.h
  src/game.h
  src/lt.h
  src/lt/lt_slot.h
  src/game/level.h
  src/game/level/goals.h
  src/renderer.h
  src/triangle.h
  src/pi.h
  src/color.h
  src/rigid_rect.h
  src/dying_rect.h
  src/game/level/lava.h
  src/wavy_rect.h
  src/game/sound_medium.h
  src/path.h
  src/mat3x3.h
  src/algo.h
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
