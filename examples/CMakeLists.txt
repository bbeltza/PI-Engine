project(pi_examples_common)

add_library(pi_examples_common STATIC
                "common/cube.c"
            )

target_link_libraries(pi_examples_common PUBLIC piengine Werror::Werror)
target_include_directories(pi_examples_common PUBLIC "./")

function(COPY_ASSET target path)
    set(INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${target}/${path}")
    set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${path}")
    add_custom_command(
        OUTPUT ${OUTPUT_PATH}
        #DEPENDS ${INPUT_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${INPUT_PATH} ${OUTPUT_PATH}
        COMMENT "Copying asset from \"${INPUT_PATH}\" to \"${OUTPUT_PATH}\"..."
        VERBATIM
    )
    set(CUSTOM_TARGET "${target}_copy_asset_${path}")
    add_custom_target(
        ${CUSTOM_TARGET}
        DEPENDS ${OUTPUT_PATH}
    )
    add_dependencies(${target} ${CUSTOM_TARGET})
endfunction()

macro(BUILD_EXAMPLE name)
    project(${name})

    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${name}/")

    add_executable(${name} ${SRCS})
    target_link_libraries(${name} pi_examples_common)
endmacro()

BUILD_EXAMPLE(logging "main.c")
BUILD_EXAMPLE(runtest "main.cpp")
BUILD_EXAMPLE(modeltest "main.cpp" "tiny_obj.cc")
BUILD_EXAMPLE(rigidbodies "main.cpp" "game.cpp" "player.cpp" "cube.cpp")
BUILD_EXAMPLE(normals "main.cpp" "app.cpp")
BUILD_EXAMPLE(spheres "main.cpp" "spheres.cpp")
BUILD_EXAMPLE(2d-obbs "main.c" "app.c")

COPY_ASSET(modeltest "guitarmodel.obj")
