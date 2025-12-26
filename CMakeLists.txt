cmake_minimum_required(VERSION 4.1.2)
project(sunflower VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-inline -fno-omit-frame-pointer")

enable_testing()

add_subdirectory(tests)
add_subdirectory(bin)

file(GLOB_RECURSE NATIVE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/native/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/native/*.hpp")
# file(GLOB_RECURSE NATIVE_STRING_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/native/string/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/native/string/*.hpp")
# file(GLOB_RECURSE MOD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/native/module/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/native/module/*.hpp")


add_library(sunflower
    header.hpp
    vec.hpp vec.cpp
    str.hpp str2.cpp
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
    sfdict.hpp sfdict.cpp
    arithmetic.hpp arithmetic.cpp
    sfclass.hpp sfclass.cpp
    environment.hpp environment.cpp

    win/sf-win.hpp win/sf-win.cpp

    # native/native.hpp native/native.cpp
    # native/putln.hpp native/putln.cpp
    # native/write.hpp native/write.cpp
    ${NATIVE_SOURCES}

    sunflower.hpp sunflower.cpp)

    
include(InstallRequiredSystemLibraries)

set(CPACK_ENABLE_COMPONENT_INSTALL ON)
set(CPACK_MONOLITHIC_INSTALL OFF)

set(CPACK_COMPONENTS_ALL Runtime)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Sunflower Runtime")
set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

set (CPACK_GENERATOR "productbuild")

set(CPACK_PACKAGE_NAME "sf")
set(CPACK_PACKAGE_VENDOR "shrehanrajsingh")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sunflower - A scripting language")
set(CPACK_PACKAGE_CONTACT "shrehanofficial@gmail.com")


include(CPack)