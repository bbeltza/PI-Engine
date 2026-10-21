macro(BUILD_SYSTEM name)
    set(PROJ piengine_${name})
    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${name}/")

    project(${PROJ})
    add_library(${PROJ} STATIC ${SRCS})
    target_link_libraries(${PROJ} PRIVATE piengine_private)
    
    list(APPEND PI_SYSTEMS ${PROJ})
endmacro()

macro(PI_BUILD_BACKEND name path)
    set(PROJ piengine_${name})
    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${path}/")

    project(${PROJ})
    add_library(${PROJ} STATIC ${SRCS})
    target_link_libraries(${PROJ} PRIVATE piengine_headers Werror::Werror SDL3::Headers)

    list(APPEND PI_SYSTEMS ${PROJ})
endmacro()

BUILD_SYSTEM(core "debug.c" "alloc.c" "main.c" "core.c")
BUILD_SYSTEM(render "render.cpp" "worldenv.cpp" "scene.cpp" "mesh.cpp" "instance.cpp" "light.cpp" "update.cpp")
BUILD_SYSTEM(physics "physics.c" "scene.c" "rigidbody.c" "collision.c" "update.c" "calc.c")

if (UNIX)
    set(PI_IO_SOURCES "io_unix.c")
elseif (WIN32)
    set(PI_IO_SOURCES "io_win32.c")
else ()
    message(FATAL_ERROR "Unsupported platform for IO. bbeltza MUST write an implementation for it. If you get this issue, please drop an issue. Thanks.")
endif()

BUILD_SYSTEM(io ${PI_IO_SOURCES})

#

check_include_file("d3d10.h" HAVE_D3D10_H)

if (HAVE_D3D10_H)
    target_compile_definitions(piengine_private INTERFACE PI_HAVE_D3D10=1)
    # Build and configure shader compiler
    project(pi_compiler_d3d10)
    add_executable(pi_compiler_d3d10 "render/d3d10/compile_d3d10_shaders.cpp")
    target_link_libraries(pi_compiler_d3d10 PRIVATE Werror::Werror)
    add_custom_command(
        OUTPUT "render/d3d10/shaders.h"
        DEPENDS "render/d3d10/shaders.hlsl"
        COMMAND pi_compiler_d3d10
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/render/d3d10/"
        COMMENT "Compiling D3D10 shaders..."
        VERBATIM
    )
    PI_BUILD_BACKEND(render_d3d10 "render/d3d10"
                "shaders.h"
                "d3d10_bootstrap.cpp"
                "d3d10_init.cpp"
                "d3d10_worldenv.cpp"
                "d3d10_scene.cpp"
                "d3d10_mesh.cpp"
                "d3d10_instbuffer.cpp"
            )
endif()

#

# find_package(OpenGL)

if (SDL_OPENGL) # Compiling with OpenGL should always be available (Even if the required GL packages are installed, unless, SDL_OPENGL is disabled...)
    target_compile_definitions(piengine_private INTERFACE PI_HAVE_GL=1)
    PI_BUILD_BACKEND(render_gl "render/gl"
                "gl_bootstrap.c"
                "gl_loader.c"
                "gl_shaders.c"
                "gl_init.c"
                "gl_worldenv.c"
                "gl_scene.c"
                "gl_mesh.c"
                "gl_instbuffer.c"
            )
endif()

#

set(PI_SYSTEMS ${PI_SYSTEMS} PARENT_SCOPE)
