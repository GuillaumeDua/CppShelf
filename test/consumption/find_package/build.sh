#!/usr/bin/env bash
set -euo pipefail

# Consumption mode: install + find_package.
#
# Self-contained:
# - configures csl
# - installs it into a dedicated directory
# - then consumes ONLY that installed directory (via CMAKE_PREFIX_PATH), so the consumer never sees the source tree.
#
# Usage:
#   bash build.sh <build-dir> [extra cmake args for the consumer...]
#
# Environment:
#   CSL_SOURCE_DIR  csl source to install (default: repo root, three levels up)
#   CMAKE_BUILD_TYPE / CMAKE_C_COMPILER / CMAKE_CXX_COMPILER  as usual

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${1:?Usage: $(basename "$0") <build-dir> [extra cmake args...]}"
shift

CSL_SOURCE_DIR="${CSL_SOURCE_DIR:-$(cd "${SCRIPT_DIR}/../../.." && pwd)}"
CSL_BUILD_DIR="${BUILD_DIR}/csl-build"
CSL_INSTALL_DIR="${BUILD_DIR}/csl-install"
CONSUMER_BUILD_DIR="${BUILD_DIR}/consumer"

echo "[find_package] installing csl from [${CSL_SOURCE_DIR}] into [${CSL_INSTALL_DIR}] ..."

# 1. configure + build + install csl into a dedicated directory
cmake \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"   \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER:-cc}"        \
    -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER:-c++}"   \
    -G Ninja                                            \
    -DCSL_TEST=OFF                                  \
    -DCSL_EXAMPLE=OFF                               \
    -DCSL_BENCHMARK=OFF                             \
    -DCSL_DOC=OFF                                   \
    -DCSL_INSTALL=ON                                \
    -S "${CSL_SOURCE_DIR}"                              \
    -B "${CSL_BUILD_DIR}"
cmake --build   "${CSL_BUILD_DIR}"
cmake --install "${CSL_BUILD_DIR}" --prefix "${CSL_INSTALL_DIR}"

echo "[find_package] consuming installed csl from [${CSL_INSTALL_DIR}] ..."

# 2. configure + build + test the consumer against the installed tree ONLY
cmake \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"   \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER:-cc}"        \
    -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER:-c++}"   \
    -G Ninja                                            \
    -DCMAKE_PREFIX_PATH="${CSL_INSTALL_DIR}"            \
    "$@"                                                \
    -S "${SCRIPT_DIR}"                                  \
    -B "${CONSUMER_BUILD_DIR}"
cmake --build "${CONSUMER_BUILD_DIR}"

ctest --parallel 4      \
    --output-on-failure \
    --test-dir "${CONSUMER_BUILD_DIR}"
