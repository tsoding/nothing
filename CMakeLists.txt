cmake_minimum_required(VERSION 3.2)
project(nothing)

if(WIN32)
  set(SDL2_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/SDL2/include")

  # Support both 32 and 64 bit builds
  if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/lib/x64/SDL2.lib;${CMAKE_SOURCE_DIR}/SDL2/lib/x64/SDL2main.lib")
  else()
    set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/lib/x86/SDL2.lib;${CMAKE_SOURCE_DIR}/SDL2/lib/x86/SDL2main.lib")
  endif()

  string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
else()
  find_package(SDL2 REQUIRED)
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(src/)

include_directories(${SDL2_INCLUDE_DIR})

add_library(system STATIC
  src/system/line_stream.c
  src/system/line_stream.h
  src/system/log.c
  src/system/log.h
  src/system/log_script.c
  src/system/log_script.h
  src/system/lt.h
  src/system/nth_alloc.c
  src/system/nth_alloc.h
  src/system/stacktrace.c
  src/system/stacktrace.h
  src/system/str.c
  src/system/str.h
  src/dynarray.h
  src/dynarray.c
  src/hashset.h
  src/hashset.c
  )

add_library(ebisp STATIC
  src/ebisp/builtins.c
  src/ebisp/builtins.h
  src/ebisp/expr.c
  src/ebisp/expr.h
  src/ebisp/gc.c
  src/ebisp/gc.h
  src/ebisp/interpreter.c
  src/ebisp/interpreter.h
  src/ebisp/parser.c
  src/ebisp/parser.h
  src/ebisp/scope.c
  src/ebisp/scope.h
  src/ebisp/std.c
  src/ebisp/std.h
  src/ebisp/tokenizer.c
  src/ebisp/tokenizer.h
  )
target_link_libraries(ebisp system)

add_executable(nothing 
  broadcast_lisp.h
  src/broadcast.c
  src/broadcast.h
  src/color.c
  src/color.h
  src/game.c
  src/game.h
  src/game/camera.c
  src/game/camera.h
  src/game/level.c
  src/game/level.h
  src/game/level/background.c
  src/game/level/background.h
  src/game/level/boxes.c
  src/game/level/boxes.h
  src/game/level/goals.c
  src/game/level/goals.h
  src/game/level/labels.c
  src/game/level/labels.h
  src/game/level/lava.c
  src/game/level/lava.h
  src/game/level/lava/wavy_rect.c
  src/game/level/lava/wavy_rect.h
  src/game/level/platforms.c
  src/game/level/platforms.h
  src/game/level/player.c
  src/game/level/player.h
  src/game/level/explosion.c
  src/game/level/explosion.h
  src/game/level/regions.c
  src/game/level/regions.h
  src/game/level/rigid_bodies.c
  src/game/level/rigid_bodies.h
  src/game/level/script.c
  src/game/level/script.h
  src/game/level_picker.c
  src/game/level_picker.h
  src/game/level_folder.h
  src/game/level_folder.c
  src/game/sound_samples.c
  src/game/sound_samples.h
  src/game/sprite_font.c
  src/game/sprite_font.h
  src/main.c
  src/math/extrema.c
  src/math/extrema.h
  src/math/mat3x3.c
  src/math/mat3x3.h
  src/math/pi.h
  src/math/point.c
  src/math/point.h
  src/math/rand.c
  src/math/rand.h
  src/math/rect.c
  src/math/rect.h
  src/math/triangle.c
  src/math/triangle.h
  src/sdl/renderer.c
  src/sdl/renderer.h
  src/sdl/texture.h
  src/sdl/texture.c
  src/ui/console.c
  src/ui/console.h
  src/ui/console_log.c
  src/ui/console_log.h
  src/ui/edit_field.c
  src/ui/edit_field.h
  src/ui/history.c
  src/ui/history.h
  src/ui/list_selector.h
  src/ui/list_selector.c
  src/ui/menu_title.h
  src/ui/menu_title.c
  src/game/level_metadata.h
  src/game/level_metadata.c
  src/game/level/level_editor/proto_rect.h
  src/game/level/level_editor/proto_rect.c
  src/game/level/level_editor.h
  src/game/level/level_editor.c
  src/game/level/level_editor/color_picker.h
  src/game/level/level_editor/color_picker.c
  src/game/level/level_editor/rect_layer.h
  src/game/level/level_editor/rect_layer.c
  src/game/level/level_editor/layer_picker.h
  src/game/level/level_editor/layer_picker.c
  src/game/level/level_editor/point_layer.h
  src/game/level/level_editor/point_layer.c
  src/game/level/level_editor/player_layer.h
  src/game/level/level_editor/player_layer.c
  src/game/level/level_editor/layer.h
  src/game/level/level_editor/layer.c
)
target_link_libraries(nothing ${SDL2_LIBRARIES} m system ebisp)

add_custom_command(
  OUTPUT broadcast_lisp.h
  COMMAND ${CMAKE_BINARY_DIR}/baker ${CMAKE_SOURCE_DIR}/src/broadcast.lisp broadcast_lisp.h broadcast_lisp_library
  DEPENDS baker src/broadcast.lisp)

add_executable(repl
  src/ebisp/repl.c
  src/ebisp/repl_runtime.c
  )
target_link_libraries(repl ${SDL2_LIBRARIES} m system ebisp)

add_executable(baker
  src/ebisp/baker.c
  )
target_link_libraries(baker m system ebisp)

add_executable(nothing_test
  test/main.c
  test/test.h
  test/tokenizer_suite.h
  )
target_link_libraries(nothing_test ${SDL2_LIBRARIES} m system ebisp)

if (UNIX)
  add_subdirectory(devtools/svg2level)
endif()

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
     -ggdb \
     -O3")
  target_link_libraries(nothing m)
endif()
if(WIN32)
  target_link_libraries(nothing Imm32 Version winmm)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sounds DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test-data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
