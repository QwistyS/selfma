cmake_minimum_required(VERSION 3.20)
project(selfma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE core/main.cpp)
set(XTRN extern)

add_subdirectory(${XTRN}/Unity)

# Include directories for core module headers
include_directories(core/inc)
include_directories(${XTRN}/stuff/inc)

set(STUFF ${XTRN}/stuff/inc/)

file(GLOB SELFMA_IMPL
  core/project.cpp
  core/container.cpp
  ${STUFF}/qwistys_alloc.c
  ${STUFF}/qwistys_avltree.c
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCE} ${SELFMA_IMPL})

target_link_libraries(${PROJECT_NAME}
  unity
)

# Test executable
set(TEST_SOURCES
    test/test_project.c
    test/runner/test_project_runner.c
    ${SELFMA_IMPL}
)

add_executable(run_tests ${TEST_SOURCES})

target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/${XTRN}/Unity
)

target_link_libraries(run_tests
    unity
)

# Ensure test files are compiled as C++
set_source_files_properties(
    test/test_project.c
    test/runner/test_project_runner.c
    PROPERTIES
    LANGUAGE CXX
)

# Add a custom target to run the tests
add_custom_target(test
    COMMAND run_tests
    DEPENDS run_tests
)
