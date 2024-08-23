cmake_minimum_required(VERSION 3.20)
project(selfma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# support clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE core/main.cpp)
set(XTRN extern)

add_subdirectory(${XTRN}/Unity)
add_subdirectory(${XTRN}/imtui)
add_subdirectory(${XTRN}/stuff)  # Add qwistys_lib as a subdirectory

# Include directories for core module headers
include_directories(core/inc)
include_directories(${XTRN}/stuff/inc)
include_directories(${XTRN}/imtui/include)
include_directories(${XTRN}/imtui/third-party)
include_directories(${XTRN}/imtui/examples)

file(GLOB SELFMA_IMPL
  core/project.cpp
  core/container.cpp
  core/selfma_api.cpp
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCE} ${SELFMA_IMPL})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    qwistys_lib
    imtui
    imtui-ncurses
    imgui-for-imtui
)

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

# Set include directories and link libraries for test executables
foreach(test_target test_project test_container)
  target_include_directories(${test_target} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
      ${CMAKE_CURRENT_SOURCE_DIR}/${XTRN}/Unity
  )
  target_link_libraries(${test_target} PRIVATE unity qwistys_lib)
  
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
