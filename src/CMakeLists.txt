macro(BUILD_SYSTEM name)
    set(PROJ piengine_${name})
    set(SRCS ${ARGN})
    list(TRANSFORM SRCS PREPEND "${name}/")

    project(${PROJ})
    add_library(${PROJ} STATIC ${SRCS})
    target_link_libraries(${PROJ} PRIVATE piengine_private)
    
    list(APPEND PI_SYSTEMS ${PROJ})
endmacro()

BUILD_SYSTEM(render "render.cpp" "scene.cpp" "mesh.cpp")
BUILD_SYSTEM(main "main.c")

set(PI_SYSTEMS ${PI_SYSTEMS} PARENT_SCOPE)
