cmake_minimum_required(VERSION 3.14)
project(SpiralFHEiO VERSION 47.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenFHE
find_package(OpenFHE REQUIRED)

# Library
add_library(spiral_fhe_io INTERFACE)
target_include_directories(spiral_fhe_io INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(spiral_fhe_io INTERFACE
    OPENFHEpke
    OPENFHEcore
    OPENFHEbinfhe
    ntl
    gmp
)

# Examples
add_executable(fhe_demo examples/fhe_demo.cpp)
target_link_libraries(fhe_demo spiral_fhe_io)

add_executable(io_demo examples/io_demo.cpp)
target_link_libraries(io_demo spiral_fhe_io)

add_executable(bridge_demo examples/bridge_demo.cpp)
target_link_libraries(bridge_demo spiral_fhe_io)
