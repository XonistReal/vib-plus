cmake_minimum_required(VERSION 3.20)
project(vib_runtime_sdl C)

add_library(vib_runtime_sdl STATIC
    sdl_backend.c
)

target_include_directories(vib_runtime_sdl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(SDL2 REQUIRED)
target_link_libraries(vib_runtime_sdl PUBLIC SDL2::SDL2 SDL2::SDL2main)

add_executable(vib_sdl_smoke
    smoke_main.c
)
target_link_libraries(vib_sdl_smoke PRIVATE vib_runtime_sdl)

