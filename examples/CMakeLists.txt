macro(BUILD_EXAMPLE name)
    project(${name})

    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${name}/")

    add_executable(${name} ${SRCS})
    target_link_libraries(${name} piengine Werror::Werror)
endmacro()

BUILD_EXAMPLE(runtest "main.cpp")
