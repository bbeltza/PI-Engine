project(pi_examples_common)

add_library(pi_examples_common STATIC
                "common/cube.c"
            )

target_link_libraries(pi_examples_common PUBLIC piengine Werror::Werror)
target_include_directories(pi_examples_common PUBLIC "./")

macro(BUILD_EXAMPLE name)
    project(${name})

    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${name}/")

    add_executable(${name} ${SRCS})
    target_link_libraries(${name} pi_examples_common)
endmacro()

BUILD_EXAMPLE(runtest "main.cpp")
BUILD_EXAMPLE(modeltest "main.cpp" "tiny_obj.cc")
BUILD_EXAMPLE(rigidbodies "main.cpp" "game.cpp" "cube.cpp")
