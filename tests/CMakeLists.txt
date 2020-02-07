set(TESTS_SOURCES
  main.cpp
  tests.cpp
)

find_package(GTest)
if (NOT ${GTEST_FOUND})
   message(FATAL_ERROR "Package GTest not found.")
endif()

add_executable(tests ${TESTS_SOURCES})

target_link_libraries(
    tests PUBLIC

    ${GTEST_LIBRARIES}
)
