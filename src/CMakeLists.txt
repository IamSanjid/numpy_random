include_directories(.)

set_property(DIRECTORY APPEND PROPERTY
    COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

    add_definitions(-DNOMINMAX)

    add_definitions(-DWIN32_LEAN_AND_MEAN)
else()
    if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        add_compile_options("-stdlib=libc++")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()
endif()

add_subdirectory(numpy)
add_library(numpy_random STATIC
    "numpy_random.cpp" 
    "numpy_random.h"
)

# Combine both static libraries
add_library(libnumpyrandom STATIC $<TARGET_OBJECTS:numpy> $<TARGET_OBJECTS:numpy_random>)
target_link_libraries(libnumpyrandom PUBLIC numpy numpy_random)

create_target_directory_groups(numpy_random)