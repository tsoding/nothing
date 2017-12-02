cmake_minimum_required(VERSION 3.2)
project(nothing)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra -pedantic -Wconversion -std=c11")

set(SOURCE_FILES
  src/main.c
  src/player.c
  src/platforms.c
  src/rect.c
)

set(HEADER_FILES
  src/player.h
  src/platforms.h
  src/rect.h
)

add_executable(nothing ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(nothing ${SDL2_LIBRARIES})
