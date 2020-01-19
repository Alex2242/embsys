cmake_minimum_required(VERSION 3.16)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(triple arm-buildroot-linux-uclibcgnueabihf)

set(toolchain $ENV{toolchain})

set(CMAKE_SYSROOT ${toolchain}/${triple}/sysroot)
# set(CMAKE_FIND_ROOT_PATH ${toolchain}/${triple}/sysroot)

# set(CMAKE_SYSROOT ${toolchain})
SET(CMAKE_FIND_ROOT_PATH ${toolchain})
# set(CMAKE_PREFIX_PATH ${toolchain})

# set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER ${toolchain}/bin/${triple}-gcc)
set(CMAKE_C_COMPILER_TARGET ${triple})
# set(CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN ${toolchain})

set(ARCH_FLAGS "-mfloat-abi=hard")
# set(CMAKE_C_FLAGS_INIT " -B${CMAKE_SYSROOT}")
# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
