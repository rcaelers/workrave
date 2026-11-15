#!/bin/bash
set -euo pipefail

configuration=${WORKRAVE_CONFIGURATION:-Release}
build="/build/qt/$configuration"
output="/build/output/qt/$configuration"
dependencies=/workspace/dependencies/conan
conan_output="/build/conan/$configuration"
generators="$conan_output/build/RelWithDebInfo/generators"


configure() {
  # Resolve the already-built locked packages and regenerate Linux host tools.
  # A cache hit for the SDK must not depend on leftover generator files.
  conan install "$dependencies" -pr:b=linux-x86_64 -pr:h=windows-x86_64 \
    --lockfile="$dependencies/windows.lock" --build=never \
    '-o:h=&:with_grpc=False' -of="$conan_output" \
    -c:a tools.graph:skip_binaries=False \
    --format=json --out-file="$conan_output/graph.json"

  cmake --fresh -S /workspace/source -B "$build" -G Ninja \
    -DCMAKE_BUILD_TYPE="$configuration" \
    -DCMAKE_INSTALL_PREFIX="$output" \
    -DCMAKE_TOOLCHAIN_FILE="$generators/conan_toolchain.cmake" \
    '-DCMAKE_MAP_IMPORTED_CONFIG_RELEASE=RelWithDebInfo;Release;' \
    '-DCMAKE_MAP_IMPORTED_CONFIG_DEBUG=RelWithDebInfo;Release;' \
    -DCMAKE_ASM_MASM_COMPILER=/opt/llvm-mingw/bin/llvm-ml \
    -DCMAKE_ASM_MASM_FLAGS=--m64 \
    -DCMAKE_FIND_USE_INSTALL_PREFIX=OFF \
    -DWORKRAVE_HOST_PROTOC=/opt/protoc/bin/protoc \
    -DWITH_UI=Qt -DWITH_GRPC=OFF -DWITH_DBUS=OFF -DWITH_TESTS=ON \
    -DRPC_CODEGEN=OFF -DWITH_AUTO_UPDATE=ON \
    -DWITH_CRASHPAD="$([[ "$configuration" == Release ]] && echo ON || echo OFF)" \
    -DWITH_UPDATER_STAGING="${WORKRAVE_STAGING:-OFF}" \
    -DWITH_CRASHPAD_STAGING="${WORKRAVE_STAGING:-OFF}" \
    -DWITH_SIGN=OFF -DWITH_SBOM=OFF -DWORKRAVE_WINDOWS_STAGE_ONLY=ON \
    -DCMAKE_PROJECT_Workrave_INCLUDE=/workspace/scripts/local/windows-precompile/precompile.cmake
}

compile() {
  cmake --build "$build" --parallel "${WORKRAVE_BUILD_JOBS:-16}"
}

stage() {
  # Rebuilds must not leave removed resources or DLLs in the handoff.
  rm -rf "$output"
  cmake --install "$build"
}

symbols() {
  if [[ "$configuration" == Release ]]; then
    sdk="/build/sdk/$(sha256sum /workspace/sdk/sdk.tar.gz | cut -c1-24)"
    if [[ ! -f "$sdk/manifest.json" ]]; then
      mkdir -p "$sdk"
      tar -xzf /workspace/sdk/sdk.tar.gz -C "$sdk"
    fi
    python3 /workspace/scripts/local/windows-symbols.py "$build" --tool /opt/dump-syms/dump_syms --sdk "$sdk"
  fi
}

handoff() {
  python3 /workspace/scripts/local/windows-qt-handoff.py export \
    --build "$build" --install "$output" --source /workspace/source \
    --archive "/workspace/handoff/$configuration.tar.gz"
}

case "${1:-all}" in
  configure|compile|stage|symbols|handoff) "$1" ;;
  all) configure; compile; stage; symbols; handoff ;;
  *) echo "Unknown Qt build phase: $1" >&2; exit 2 ;;
esac
