cmake_minimum_required(VERSION 3.10)

project(Embsys VERSION 0.0.1 LANGUAGES C)

include(../CMakeLists.txt)

add_subdirectory(../../src/network ../../build/arm/network)
add_subdirectory(../../src/client ../../build/arm/client)
add_subdirectory(../../src/camera ../../build/arm/camera)
add_subdirectory(../../src/server ../../build/arm/server)
add_subdirectory(../../src/utils ../../build/arm/utils)