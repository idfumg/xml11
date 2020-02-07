cmake_minimum_required(VERSION 3.3.2)
project(xml11)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(CXXFLAGS "-Wold-style-cast -pedantic-errors -Wall -Werror -Wextra -ansi -Wshadow -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -I/usr/include/libxml2 -I./xml11 -lxml2")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   set(CXXFLAGS "-fno-omit-frame-pointer -g -fsanitize=address -Wold-style-cast -pedantic-errors -Wall -Werror -Wextra -ansi -Wshadow -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -I/usr/include/libxml2 -I./xml11 -I. -lxml2")
 endif()

 if (NOT CONFIGURED_ONCE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLAGS}"
       CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../xml11")
add_subdirectory("tests")

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
