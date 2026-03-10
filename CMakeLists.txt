cmake_minimum_required(VERSION 3.16)
project(KiKigame C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(kikigame src/main.c)
target_link_libraries(kikigame PRIVATE raylib)

if(APPLE)
  target_link_libraries(kikigame PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
