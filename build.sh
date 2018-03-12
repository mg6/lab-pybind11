#!/bin/bash

set -euo pipefail

build() {
    mkdir -p build
    pushd build
    cmake ..
    # override Python version with: cmake .. -DPYBIND11_PYTHON_VERSION=2.7
    make
    popd
}

test_() {
    pushd build
    python3 -c 'import example; help(example)'
}

clean() {
    rm -rf build
}

main() {
    ! [ $# -gt 0 ] \
        && build \
        || for action in "${@}"; do
            local suffix=""
            declare -f "$action" > /dev/null || suffix=_
            ( set -x ; "${action}${suffix}" )
        done
}

main "${@}"
