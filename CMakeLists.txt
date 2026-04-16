cmake_minimum_required(VERSION 3.24)
project(Pac-Man)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-3.4.2
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG        release-3.4.0
  OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(SDL3 SDL3_image)

add_executable(Pac-Man
  game.cpp
  game.hpp
  texture_manager.cpp
  texture_manager.hpp
  game_map.cpp
  game_map.hpp)
target_link_libraries(Pac-Man PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)

add_custom_command(TARGET Pac-Man POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:Pac-Man>
)

add_custom_command(TARGET Pac-Man POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE_DIR:Pac-Man>
)

add_custom_command(TARGET Pac-Man POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Pac-Man>/assets
)