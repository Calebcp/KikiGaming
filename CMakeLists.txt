cmake_minimum_required(VERSION 3.16)
project(KiKigame C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(kikigame
  src/main.c
  src/assets.c
  src/game_state.c
  src/rendering.c
  src/levels.c
)
target_link_libraries(kikigame PRIVATE raylib)

add_custom_command(TARGET kikigame POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  $<TARGET_FILE_DIR:kikigame>/assets
)

if(APPLE)
  target_link_libraries(kikigame PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

if(WIN32)
  target_link_libraries(kikigame PRIVATE winmm)
endif()
