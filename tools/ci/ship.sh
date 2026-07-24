#!/bin/bash

build_ship() {
    local previous_dir
    previous_dir=$(pwd)

    cd "${SCRIPTS_DIR}/ship"
    cargo build --release

    if [ -x "${SCRIPTS_DIR}/ship/target/release/ship.exe" ]; then
        export SHIP="${SCRIPTS_DIR}/ship/target/release/ship.exe"
    else
        export SHIP="${SCRIPTS_DIR}/ship/target/release/ship"
    fi

    cd "${previous_dir}"
}

run_ship() {
    if [ -z "${SHIP:-}" ]; then
        build_ship
    fi

    "${SHIP}" "$@"
}
