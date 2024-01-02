#!/usr/bin/env bash


set -e


declare -r SRC_DIR="$PWD"
declare -r BUILD_DIR="$SRC_DIR/_build"
declare -r GENERATOR="Ninja"
declare -r BUILD_TYPE="Debug"
declare -ar CMAKE_CONFIG_ARGS=( \
    -S "$SRC_DIR" \
    -B "$BUILD_DIR" \
    -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
)


print_usage()
{
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  When called without options cmake will configure and build the project"
    echo "  -h, --help              print this message"
    echo "  -b, --build [target]    configure and build the project, optionally can specify target"
    echo "  -f, --fresh             remove existing cache files"
    echo "  -c, --clean [target]    clean the cmake project, optionally can specify target"
    echo "  -d, --delete            remove the build directory"
    exit
}


while true; do

    case "$1" in

        ""|"-b"|"--build")
            cmake "${CMAKE_CONFIG_ARGS[@]}"
            cmake --build "$BUILD_DIR"
            ;;

        "-f"|"--fresh")
            cmake "${CMAKE_CONFIG_ARGS[@]}" --fresh
            ;;

        "-c"|"--clean") 
            cmake --build "$BUILD_DIR" --target clean
            ;;

        "-d"|"--delete") 
            rm -rf "$BUILD_DIR"
            ;;

        "-h"|"--help"|*)
            print_usage
            ;;
    esac
    
    [[ $# -eq 0 ]] && break
    shift

done
