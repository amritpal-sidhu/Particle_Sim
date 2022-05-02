#!/bin/bash

build_type="-D CMAKE_BUILD_TYPE=Debug"
source_dir="${PWD}"
build_dir="${PWD}/_build"
cmake_cache_file="${build_dir}/CMakeCache.txt"
toolchain_file="${PWD}/cmake/x86_64-w64-mingw32.cmake"
log_level="--log-level=VERBOSE"


check_toolchain()
{
    cached_toolchain=$(grep CMAKE_TOOLCHAIN_FILE "$cmake_cache_file" | cut -d "=" -f2)
    return [ "$toolchain_file" = "$cached_toolchain" ]
}


if [[ -f $cmake_cache_file && check_toolchain ]]; then
    cmake -S $source_dir -B $build_dir $build_type $log_level
else
    cmake --toolchain $toolchain_file -S $source_dir -B $build_dir  $build_type $log_level
fi

cmake --build $build_dir
