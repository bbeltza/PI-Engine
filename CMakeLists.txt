cmake_minimum_required(VERSION 3.12)

set(BUILD_SHARED_LIBS OFF)

project(piengine)
project(piengine_private)
project(piengine_headers)

add_library(piengine INTERFACE)
add_library(piengine_private INTERFACE)
add_library(piengine_headers INTERFACE)

option(SDL_VENDORED "Use SDL vendored library (the one inside the root direcotry's \"SDL/\" folder)" ON)

if (SDL_VENDORED)
    # Setup SDL if using vendored libs (Only need the VIDEO subsystem for now)

    set(SDL_VIDEO ON)

    set(SDL_AUDIO OFF)
    set(SDL_GPU OFF)
    set(SDL_RENDER OFF)
    set(SDL_CAMERA OFF)
    set(SDL_JOYSTICK OFF)
    set(SDL_HAPTIC OFF)
    set(SDL_HIDAPI OFF)
    set(SDL_POWER OFF)
    set(SDL_SENSOR OFF)
    set(SDL_DIALOG OFF)
    set(SDL_TRAY OFF)
    set(SDL_NOTIFICATION OFF)

    add_subdirectory(SDL EXCLUDE_FROM_ALL)
else()
    # Otherwise...

    find_package(SDL3)
    if (NOT SDL3_FOUND)
        # TODO: Handle packaged SDL3 not found case.
        message(FATAL_ERROR "Package SDL3 is not found... (Insert more message)")
    endif()
endif()


#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")


include(CheckIncludeFile)
include(werror)

target_include_directories(piengine_headers INTERFACE "${CMAKE_SOURCE_DIR}/include/")
target_link_libraries(piengine_private INTERFACE piengine_headers Werror::Werror)
target_link_libraries(piengine_private INTERFACE SDL3::SDL3)

add_subdirectory(src)

target_link_libraries(piengine INTERFACE ${PI_SYSTEMS})
target_link_libraries(piengine INTERFACE piengine_headers)

#

add_subdirectory(examples)
