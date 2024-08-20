cmake_minimum_required(VERSION 3.20)
project(selfma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE core/main.cpp)
set(XTRN extern)

file(GLOB SELFMA_IMPL
  core/project.cpp
)

# Include directories for core module headers
include_directories(core/inc)
include_directories(${XTRN}/stuff/inc)
# include_directories(${XTRN}/TermOx/include)


# add_subdirectory(extern/TermOx)
add_executable(${PROJECT_NAME} ${SOURCE} ${SELFMA_IMPL})

target_link_libraries(${PROJECT_NAME}
  # TermOx
)
