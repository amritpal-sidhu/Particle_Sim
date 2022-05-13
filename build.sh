#!/bin/bash

set -e


build_type="Debug"
source_dir="${PWD}"
build_dir="${PWD}/_build"
generator="MinGW Makefiles"
options=("-D CMAKE_BUILD_TYPE=$build_type")

case $1 in

"rm")  rm -rf $build_dir
;;

"clean")  cmake --build $build_dir --target clean
;;

*)
    cmake -G "$generator" -S $source_dir -B $build_dir ${options[*]}
    cmake --build $build_dir
;;

esac
