#!/bin/bash

build_ship() {
    local previous_dir
    previous_dir=$(pwd)

    cd "${SCRIPTS_DIR}/ship"
    cargo build --release

    local target_dir="${CARGO_TARGET_DIR:-${SCRIPTS_DIR}/ship/target}"
    if [ -x "${target_dir}/release/ship.exe" ]; then
        export SHIP="${target_dir}/release/ship.exe"
    else
        export SHIP="${target_dir}/release/ship"
    fi

    cd "${previous_dir}"
}

run_ship() {
    if [ -z "${SHIP:-}" ]; then
        build_ship
    fi

    "${SHIP}" "$@"
}
