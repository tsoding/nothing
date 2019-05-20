cmake_minimum_required(VERSION 3.2)
project(nothing)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
include_directories(${LIBXML2_INCLUDE_DIRS})

add_executable(svg2level
  ./main.c
  )
target_link_libraries(svg2level ${LIBXML2_LIBRARIES})
