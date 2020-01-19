cmake_minimum_required(VERSION 3.10)

project(Embsys VERSION 0.0.1 LANGUAGES C)

include(../CMakeLists.txt)

add_subdirectory(../../src/network ../../build/x86/network)
add_subdirectory(../../src/client ../../build/x86/client)
add_subdirectory(../../src/camera ../../build/x86/camera)
add_subdirectory(../../src/server ../../build/x86/server)