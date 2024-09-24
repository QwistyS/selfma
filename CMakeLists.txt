cmake_minimum_required(VERSION 3.20)
project(selfma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(STORAGE_FOLDER_PATH "${CMAKE_SOURCE_DIR}/storage")
add_compile_definitions(STORAGE_PATH="${STORAGE_FOLDER_PATH}")

# support clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Cross-compilation settings
if(CMAKE_CROSSCOMPILING)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
  
  # Ensure we're using the correct toolchain
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_RC_COMPILER llvm-rc)
  
  # Set the target
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-w64-mingw32")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target x86_64-w64-mingw32")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -target x86_64-w64-mingw32 -fuse-ld=lld")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # For GCC and Clang
    add_compile_options(-Wno-ignored-attributes)
    # add_definitions(-DSTATIC_API)
    add_definitions(-DUSE_GNU_PACKED_ATTRIBUTE)
elseif(MSVC)
    # For Microsoft Visual C++
    add_definitions(-DUSE_MSVC_PRAGMA_PACK)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DSTATIC_API)
    add_compile_options(/wd4996)
else()
    message(WARNING "Unknown compiler, packed structs may not work as expected")
endif()


set(XTRN extern)

add_subdirectory(${XTRN}/Unity)
add_subdirectory(${XTRN}/stuff)  # Add qwistys_lib as a subdirectory

# Include directories for core module headers
include_directories(core/inc)
include_directories(${XTRN}/stuff/inc)

file(GLOB SELFMA_SOURCES
  core/project.cpp
  core/container.cpp
  core/selfma_api.cpp
  core/selfma_file.cpp
  core/selfma.cpp
)

# Create selfma as a static library
add_library(${PROJECT_NAME} STATIC ${SELFMA_SOURCES})

# Use PRIVATE linkage for qwistys_lib
target_link_libraries(${PROJECT_NAME} PRIVATE 
    qwistys_lib
)

# Ensure all test files are compiled as C++
set_source_files_properties(
    test/runner/trun_project.c
    test/runner/trun_container.c
    test/runner/trun_selfma.c
    PROPERTIES
    LANGUAGE CXX
)

# Test executables
add_executable(test_project 
    test/runner/trun_project.c
    test/test_project.cpp
)

add_executable(test_container
    test/runner/trun_container.c
    test/test_container.cpp
)

add_executable(test_selfma
    test/runner/trun_selfma.c
    test/test_selfma.cpp
)

# Set include directories and link libraries for test executables
foreach(test_target test_project test_container test_selfma)
  target_include_directories(${test_target} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
      ${CMAKE_CURRENT_SOURCE_DIR}/${XTRN}/Unity
  )
  target_link_libraries(${test_target} PRIVATE unity ${PROJECT_NAME})
  
  # Ensure test files are compiled as C++
  set_target_properties(${test_target} PROPERTIES LINKER_LANGUAGE CXX)
  set_source_files_properties(${test_target} PROPERTIES LANGUAGE CXX)
endforeach()

# Add a custom target to run all tests
add_custom_target(run_all_tests
    COMMAND test_project
    COMMAND test_container
    COMMAND test_selfma
    DEPENDS test_project test_container test_selfma
)

# Install rules
install(TARGETS ${PROJECT_NAME} qwistys_lib
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY core/inc/ 
        DESTINATION include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h")

# Export the library targets
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME})

# Create a Config file
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME})

# Install the Config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME})
