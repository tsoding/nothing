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

add_executable(nothing 
  src/color.c
  src/game.c
  src/game/camera.c
  src/ui/edit_field.c
  src/game/level.c
  src/game/level/background.c
  src/game/level/boxes.c
  src/game/level/goals.c
  src/game/level/labels.c
  src/game/level/lava.c
  src/game/level/lava/wavy_rect.c
  src/game/level/physical_world.c
  src/game/level/platforms.c
  src/game/level/player.c
  src/game/level/player/dying_rect.c
  src/game/level/player/rigid_rect.c
  src/game/level/solid.c
  src/game/sound_samples.c
  src/game/sprite_font.c
  src/main.c
  src/math/mat3x3.c
  src/math/point.c
  src/math/rand.c
  src/math/rect.c
  src/math/triangle.c
  src/ebisp/expr.c
  src/ebisp/interpreter.c
  src/ebisp/parser.c
  src/ebisp/tokenizer.c
  src/ebisp/gc.c
  src/ebisp/scope.c
  src/sdl/renderer.c
  src/system/error.c
  src/system/lt.c
  src/system/lt/lt_adapters.c
  src/system/lt/lt_slot.c
  src/ui/console.c
  src/ui/console_log.c
  src/str.c
  src/ui/history.c
  src/game/level/regions.c
  src/color.h
  src/game.h
  src/game/camera.h
  src/ui/edit_field.h
  src/game/level.h
  src/game/level/background.h
  src/game/level/boxes.h
  src/game/level/goals.h
  src/game/level/labels.h
  src/game/level/lava.h
  src/game/level/lava/wavy_rect.h
  src/game/level/physical_world.c
  src/game/level/platforms.h
  src/game/level/player.h
  src/game/level/player/dying_rect.h
  src/game/level/player/rigid_rect.h
  src/game/level/solid.h
  src/game/sound_samples.h
  src/game/sprite_font.h
  src/math/mat3x3.h
  src/math/pi.h
  src/math/point.h
  src/math/rand.h
  src/math/rect.h
  src/math/triangle.h
  src/ebisp/builtins.c
  src/ebisp/builtins.h
  src/ebisp/expr.h
  src/ebisp/gc.h
  src/ebisp/scope.h
  src/ebisp/interpreter.h
  src/ebisp/parser.h
  src/ebisp/tokenizer.h
  src/sdl/renderer.h
  src/system/error.h
  src/system/lt.h
  src/system/lt/lt_adapters.h
  src/system/lt/lt_slot.h
  src/ui/console.h
  src/ui/console_log.h
  src/str.h
  src/ui/history.h
  src/game/level/regions.h
  src/system/line_stream.h
  src/system/line_stream.c
  src/game/level/script.h
  src/game/level/script.c
  src/system/log.h
  src/system/log.c
  src/system/nth_alloc.h
  src/system/nth_alloc.c
)

add_executable(repl
  src/ebisp/builtins.c
  src/ebisp/builtins.h
  src/ebisp/expr.c
  src/ebisp/expr.h
  src/ebisp/interpreter.c
  src/ebisp/interpreter.h
  src/ebisp/parser.c
  src/ebisp/parser.h
  src/ebisp/scope.c
  src/ebisp/scope.h
  src/ebisp/tokenizer.c
  src/ebisp/tokenizer.h
  src/ebisp/repl.c
  src/system/lt.c
  src/system/lt.h
  src/system/lt/lt_adapters.c
  src/system/lt/lt_adapters.h
  src/system/lt/lt_slot.c
  src/system/lt/lt_slot.h
  src/system/error.c
  src/system/error.h
  src/ebisp/gc.h
  src/ebisp/gc.c
  src/str.h
  src/str.c
  src/system/log.h
  src/system/log.c
  src/system/nth_alloc.h
  src/system/nth_alloc.c
  )
add_executable(nothing_test
  src/ebisp/builtins.c
  src/ebisp/builtins.h
  src/ebisp/expr.c
  src/ebisp/expr.h
  src/ebisp/interpreter.c
  src/ebisp/interpreter.h
  src/ebisp/parser.c
  src/ebisp/parser.h
  src/ebisp/scope.c
  src/ebisp/scope.h
  src/ebisp/tokenizer.c
  src/ebisp/tokenizer.h
  src/ebisp/gc.h
  src/ebisp/gc.c
  src/system/error.c
  src/system/error.h
  src/system/lt.c
  src/system/lt.h
  src/system/lt/lt_adapters.c
  src/system/lt/lt_adapters.h
  src/system/lt/lt_slot.c
  src/system/lt/lt_slot.h
  src/str.h
  src/str.c
  test/main.c
  test/test.h
  test/tokenizer_suite.h
  src/system/log.h
  src/system/log.c
  src/system/nth_alloc.h
  src/system/nth_alloc.c
  )
target_link_libraries(nothing ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
target_link_libraries(nothing_test ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
target_link_libraries(repl ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})

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
     -Wswitch \
     -Wmissing-field-initializers \
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
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test-data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
