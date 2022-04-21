#!/bin/bash

cmake -D CMAKE_TOOLCHAIN_FILE=Third-Party/glfw/CMake/x86_64-w64-mingw32.cmake -S . -B build
cmake --build build
