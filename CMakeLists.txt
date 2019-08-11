cmake_minimum_required(VERSION 3.2)
project(nothing)

if(WIN32)
  if(MINGW)
    add_compile_definitions(SDL_MAIN_HANDLED) # https://stackoverflow.com/a/25089610
    add_compile_definitions(__USE_MINGW_ANSI_STDIO) # https://github.com/trink/symtseries/issues/49#issuecomment-160843756
    # Support both 32 and 64 bit builds
    if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
      set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32/include/SDL2")
      set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32/lib/libSDL2.a;${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32/lib/libSDL2main.a")
    else()
      set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/SDL2/i686-w64-mingw32/include/SDL2")
      set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/i686-w64-mingw32/lib/libSDL2.a;${CMAKE_SOURCE_DIR}/SDL2/i686-w64-mingw32/lib/libSDL2main.a")
    endif()
	  
  else()
    set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/SDL2/include")

    # Support both 32 and 64 bit builds
    if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
      set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/lib/x64/SDL2.lib;${CMAKE_SOURCE_DIR}/SDL2/lib/x64/SDL2main.lib")
    else()
      set(SDL2_LIBRARIES "${CMAKE_SOURCE_DIR}/SDL2/lib/x86/SDL2.lib;${CMAKE_SOURCE_DIR}/SDL2/lib/x86/SDL2main.lib")
    endif()

  endif()
  string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
  
else()
  find_package(SDL2 REQUIRED)
endif()

if("${SDL2_LIBRARIES}" STREQUAL "")
    message(WARNING "SDL2_LIBRARIES wasn't set, manually setting to SDL2::SDL2")
    set(SDL2_LIBRARIES "SDL2::SDL2")
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(src/)

include_directories(${SDL2_INCLUDE_DIRS})

add_library(system STATIC
  src/system/line_stream.c
  src/system/line_stream.h
  src/system/log.c
  src/system/log.h
  src/system/log_script.c
  src/system/log_script.h
  src/system/lt.h
  src/system/lt_adapters.h
  src/system/lt_adapters.c
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
  src/system/file.h
  src/system/file.c
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
  src/ui/wiggly_text.h
  src/ui/wiggly_text.c
  src/ui/slider.h
  src/ui/slider.c
  src/game/level_metadata.h
  src/game/level_metadata.c
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
  src/game/level/level_editor/label_layer.c
  src/game/level/level_editor/label_layer.h
  src/game/level/level_editor/background_layer.c
  src/game/level/level_editor/background_layer.h
  src/game/level/level_editor/undo_history.h
  src/game/level/level_editor/undo_history.c
)
target_link_libraries(nothing ${SDL2_LIBRARIES} system ebisp)

add_executable(repl
  src/ebisp/repl.c
  src/ebisp/repl_runtime.c
  )
target_link_libraries(repl ${SDL2_LIBRARIES} system ebisp)

add_executable(nothing_test
  test/main.c
  test/test.h
  test/tokenizer_suite.h
  )
target_link_libraries(nothing_test ${SDL2_LIBRARIES} system ebisp)

if(("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))
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
	   -fno-strict-aliasing \
     -pedantic \
     -std=c11 \
     -ggdb \
     -O3")
  target_link_libraries(nothing m)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} \
    /Wall \
    /WX \
    /wd4127 \
    /wd4201 \
    /wd4204 \
    /wd4255 \
    /wd4389 \
    /wd4668 \
    /wd4702 \
    /wd4710 \
    /wd4777 \
    /wd4820 \
    /wd5045 \
    /D \"_CRT_SECURE_NO_WARNINGS\"")
endif()
if(MINGW)
  target_link_libraries(nothing hid setupapi Imm32 Version winmm)
elseif(WIN32)
  target_link_libraries(nothing Imm32 Version winmm)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test-data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
