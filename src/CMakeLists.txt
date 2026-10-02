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

BUILD_SYSTEM(render "render.cpp" "worldenv.cpp" "scene.cpp" "mesh.cpp" "instance.cpp" "update.cpp")
BUILD_SYSTEM(main "main.c" "core.c")

#

check_include_file("d3d10.h" HAVE_D3D10_H)

if (HAVE_D3D10_H)
    target_compile_definitions(piengine_private INTERFACE PI_HAVE_D3D10=1)
    PI_BUILD_BACKEND(render_d3d10 "render/d3d10"
                "d3d10_bootstrap.cpp"
                "d3d10_init.cpp"
                "d3d10_worldenv.cpp"
                "d3d10_scene.cpp"
                "d3d10_mesh.cpp"
                "d3d10_instbuffer.cpp"
            )
    # Build and configure shader compiler
    project(pi_compiler_d3d10)
    add_executable(pi_compiler_d3d10 "render/d3d10/compile_d3d10_shaders.cpp")
    target_link_libraries(pi_compiler_d3d10 PRIVATE Werror::Werror)
    add_custom_target(
            pi_compile_d3d10_shaders
            COMMAND pi_compiler_d3d10
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/render/d3d10/"
            DEPENDS "render/d3d10/shaders.hlsl"
                    "render/d3d10/shaders.h"
            COMMENT "Compiling D3D10 shaders..."
            VERBATIM
        )
    add_dependencies(
        piengine_render_d3d10
        pi_compile_d3d10_shaders
    )
endif()

#

# find_package(OpenGL)

if (SDL_OPENGL) # Compiling with OpenGL should always be available (Even if the required GL packages are installed, unless, SDL_OPENGL is disabled...)
    target_compile_definitions(piengine_private INTERFACE PI_HAVE_GL=1)
    PI_BUILD_BACKEND(render_gl "render/gl"
                "gl_bootstrap.c"
            )
endif()

#

set(PI_SYSTEMS ${PI_SYSTEMS} PARENT_SCOPE)
