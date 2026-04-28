#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${1:?Usage: $(basename "$0") <build-dir> [extra cmake args...]}"
shift

cmake \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"   \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER:-cc}"        \
    -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER:-c++}"   \
    -G Ninja                                            \
    "$@"                                                \
    -S "${SCRIPT_DIR}"                                  \
    -B "${BUILD_DIR}"

cmake --build "${BUILD_DIR}"

ctest --parallel 4      \
    --output-on-failure \
    --test-dir "${BUILD_DIR}"
