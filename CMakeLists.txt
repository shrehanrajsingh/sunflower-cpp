cmake_minimum_required(VERSION 3.10.0)
project(sunflower VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-inline -fno-omit-frame-pointer")

enable_testing()

add_subdirectory(tests)

add_library(sunflower
    header.hpp
    vec.hpp vec.cpp
    str.hpp str.cpp
    tokenize.hpp tokenize.cpp
    expr.hpp expr.cpp
    stmt.hpp stmt.cpp
    object.hpp object.cpp
    const.hpp const.cpp
    module.hpp module.cpp
    memutil.hpp memutil.cpp
    func.hpp func.cpp
    ast.hpp ast.cpp
    sfarray.hpp sfarray.cpp
    sr.hpp sr.cpp
    sunflower.hpp sunflower.cpp)

