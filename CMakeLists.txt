cmake_minimum_required(VERSION 3.20)
project(selfma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE core/main.cpp)
set(CORE_SRC)
set(XTRN extern)
# Include directories for core module headers
include_directories(core/inc)
include_directories(${XTRN}/stuff/inc)
# include_directories(${XTRN}/TermOx/include)


# add_subdirectory(extern/TermOx)
add_executable(${PROJECT_NAME} ${SOURCE} ${CORE_SRC})

target_link_libraries(${PROJECT_NAME}
  # TermOx
)
