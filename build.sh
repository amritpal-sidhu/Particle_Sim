#!/usr/bin/env bash


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

get_target()
{
    local -r optglob='@(-h|--help|-b|--build|-f|--fresh|-c|--clean|-d|--delete)'
    unset -v TGT

    [[ $# -eq 0 ]] && return
    [[ "$1" == $optglob ]] || TGT="--target $1"
}


while

    case "$1" in

        ""|"-b"|"--build")
            get_target ${2:-}; declare -I TGT
            cmake "${CMAKE_CONFIG_ARGS[@]}"
            cmake --build "$BUILD_DIR" ${TGT:-}
            ;;

        "-f"|"--fresh")
            cmake "${CMAKE_CONFIG_ARGS[@]}" --fresh
            ;;

        "-c"|"--clean") 
            get_target ${2:-}; declare -I TGT
            cmake --build "$BUILD_DIR" ${TGT:-} -- -t clean
            ;;

        "-d"|"--delete") 
            rm -rf "$BUILD_DIR"
            ;;

        "-h"|"--help"|*)
            print_usage
            ;;
    esac

    shift ${TGT:+2}

do [[ $# -ne 0 ]]; done
