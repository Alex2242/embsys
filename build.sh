#!/bin/bash

set -o errexit

function script_usage() {
    cat << EOF
Usage:
     -h|--help                      Displays this help
    -ao|--arm-only                  Compile only for arm
    -tp|--toolchain-path <path>     Enable compilation for arm using the specified toolchain     
EOF
}

function parse_params() {
    local param
    while [[ $# -gt 0 ]]; do
        param="$1"
        pn="$2"
        shift
        case $param in
            -h|--help)
                script_usage
                exit 0
                ;;
            -ao|--arm-only)
                ao=true
                ;;
            -tp|--toolchain-path)
                tp="$pn"
                shift
                ;;
            *)
                script_exit "Invalid parameter was provided: $param" 1
                ;;
        esac
    done
}




function build_x86() {
    mkdir -p build/x86
    cd build/x86
    cmake -DCMAKE_BUILD_TYPE=Debug ../../cmake/x86
    cmake --build .
    cd ../..
}

function build_arm() {
    mkdir -p build/arm
    cd build/arm
    toolchain=$tp cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake ../../cmake/arm
    cmake --build .
    cd ../..
}

function main() {
    parse_params "$@"
    
    if [[ -n ${tp-} ]]; then
        build_arm
    fi

    if [[ -z ${ao-} ]]; then
        build_x86
    fi
}

main "$@"
