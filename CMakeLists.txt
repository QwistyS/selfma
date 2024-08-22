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

target_link_libraries(${PROJECT_NAME} )

# Ensure all test files are compiled as C++
set_source_files_properties(
    test/runner/trun_project.c
    test/runner/trun_container.c
    PROPERTIES
    LANGUAGE CXX
)

# Test executables
add_executable(test_project 
    test/runner/trun_project.c
    test/test_project.cpp
    ${SELFMA_IMPL}
)

add_executable(test_container
    test/runner/trun_container.c
    test/test_container.cpp
    ${SELFMA_IMPL}
)

# add_executable(test_selfma
#     test/runner/test_selfma.c
#     test/test_selfma.c
#     ${SELFMA_IMPL}
# )

# Set include directories for test executables
foreach(test_target test_project test_container)
  target_include_directories(${test_target} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
      ${CMAKE_CURRENT_SOURCE_DIR}/${XTRN}/Unity
  )
  target_link_libraries(${test_target} unity)
  
  # Ensure test files are compiled as C++
  set_target_properties(${test_target} PROPERTIES LINKER_LANGUAGE CXX)
  set_source_files_properties(${test_target} PROPERTIES LANGUAGE CXX)
endforeach()

# Add a custom target to run all tests
add_custom_target(run_all_tests
    COMMAND test_project
    COMMAND test_container
    DEPENDS test_project test_container
)
