#!/usr/bin/env bash


declare -r SRC_DIR="$PWD"
declare -r BUILD_DIR="$SRC_DIR/_build"
declare -r GENERATOR="Ninja Multi-Config"
declare -r CONFIG="Debug"
declare -r TOP_LEVEL_INCS="cmake/ParticleSimBuildModule.cmake"

declare -ar PROJ_GEN_ARGS=( \
    -S "$SRC_DIR" \
    -B "$BUILD_DIR" \
    -G "$GENERATOR" \
    -DCMAKE_CONFIGURATION_TYPES="$CONFIG" \
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="$TOP_LEVEL_INCS" \
)
declare -ar BUILD_ARGS=( \
    --build "$BUILD_DIR" \
)


print_usage()
{
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  When called without options cmake will configure and build the project"
    echo "  -h, --help              print this message"
    echo "  -i, --info              dumps system information"
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


first=1

while [[ $# -ne 0 || $first ]]; do

    case "$1" in

        ""|"-b"|"--build")
            get_target ${2:-}; declare -I TGT
            cmake "${PROJ_GEN_ARGS[@]}"
            cmake "${BUILD_ARGS[@]}" ${TGT:-}
            ;;

        "-f"|"--fresh")
            cmake "${PROJ_GEN_ARGS[@]}" --fresh
            ;;

        "-c"|"--clean") 
            get_target ${2:-}; declare -I TGT
            cmake "${BUILD_ARGS[@]}" ${TGT:-} -- -t clean
            ;;

        "-d"|"--delete") 
            rm -rf "$BUILD_DIR"
            ;;

        "-i"|"--info")
            cmake "${PROJ_GEN_ARGS[@]}"
            ;;

        "-h"|"--help"|*)
            print_usage
            ;;
    esac

    shift ${TGT:+2}
    unset first
done
