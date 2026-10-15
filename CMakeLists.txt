cmake_minimum_required(VERSION 3.20)
project(mWindows LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_library(mWindows STATIC 
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(mWindows PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(mWindows PUBLIC
    UNICODE
    _UNICODE
)