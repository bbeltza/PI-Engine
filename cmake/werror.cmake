set(WERROR_LIB __werror_target)

project(${WERROR_LIB})
add_library(${WERROR_LIB} INTERFACE)

if (MSVC)
    set(WERROR_FLAGS "/WX")
else()
    set(WERROR_FLAGS "-Werror")
endif()

target_compile_options(${WERROR_LIB} INTERFACE ${WERROR_FLAGS})

add_library(Werror::Werror ALIAS ${WERROR_LIB})
