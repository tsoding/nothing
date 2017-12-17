cmake_minimum_required(VERSION 3.2)
project(nothing)

if(WIN32)
  # Conan integration
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

set(SOURCE_FILES
  src/main.c
  src/player.c
  src/platforms.c
  src/rect.c
  src/point.c
  src/camera.c
  src/error.c
  src/game.c
  src/lt.c
  src/lt/lt_slot.c
)

set(HEADER_FILES
  src/player.h
  src/platforms.h
  src/rect.h
  src/point.h
  src/camera.h
  src/error.h
  src/game.h
  src/lt.h
  src/lt/lt_slot.h
)

add_executable(nothing ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(nothing ${SDL2_LIBRARY})

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
