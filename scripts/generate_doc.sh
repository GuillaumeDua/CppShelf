#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

# --- argument parsing ---
DOXYGEN_VERSION=
DOXYGEN_AWESOME_CSS_VERSION=

while [[ $# -gt 0 ]]; do
    case "$1" in
        --doxygen-version)         DOXYGEN_VERSION="$2";             shift 2 ;;
        --doxygen-awesome-version) DOXYGEN_AWESOME_CSS_VERSION="$2"; shift 2 ;;
        *) echo "Unknown argument: $1" >&2; exit 1 ;;
    esac
done

# strip optional leading 'v'
DOXYGEN_VERSION="${DOXYGEN_VERSION#v}"
DOXYGEN_AWESOME_CSS_VERSION="${DOXYGEN_AWESOME_CSS_VERSION#v}"

# --- helpers ---
http_get() {
    if command -v curl &>/dev/null; then
        curl -fsSL "$1"
    else
        wget -qO- "$1"
    fi
}

github_latest_tag() {
    http_get "https://api.github.com/repos/$1/releases/latest" \
        | grep '"tag_name"' | head -1 \
        | sed 's/.*"tag_name": *"\([^"]*\)".*/\1/'
}

# --- Defaults -> latest GitHub releases ---
# ex: Release_1_13_2 -> 1.13.2
if [[ -z "${DOXYGEN_VERSION}" ]]; then
    tag=$(github_latest_tag "doxygen/doxygen")
    DOXYGEN_VERSION="${tag#Release_}"
    DOXYGEN_VERSION="${DOXYGEN_VERSION//_/.}"
fi

# ex: v2.3.4 -> 2.3.4
if [[ -z "${DOXYGEN_AWESOME_CSS_VERSION}" ]]; then
    tag=$(github_latest_tag "jothepro/doxygen-awesome-css")
    DOXYGEN_AWESOME_CSS_VERSION="${tag#v}"
fi

echo "doxygen ${DOXYGEN_VERSION} / doxygen-awesome-css ${DOXYGEN_AWESOME_CSS_VERSION}"

# --- doxygen ---
installed_doxygen_version() {
    command -v doxygen &>/dev/null && doxygen --version || true
}

fetch_doxygen() {
    local tmpdir
    tmpdir=$(mktemp -d)
    trap 'rm -rf "${tmpdir}"' RETURN

    local url="https://github.com/doxygen/doxygen/releases/download/Release_${DOXYGEN_VERSION//./_}/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz"
    echo "Downloading doxygen ${DOXYGEN_VERSION}..."
    wget -q "${url}" -O "${tmpdir}/doxygen.tar.gz"
    tar xzf "${tmpdir}/doxygen.tar.gz" -C "${tmpdir}"
    sudo make -C "${tmpdir}/doxygen-${DOXYGEN_VERSION}" install
}

if [[ "$(installed_doxygen_version)" == "${DOXYGEN_VERSION}" ]]; then
    echo "doxygen ${DOXYGEN_VERSION} already installed, skipping."
else
    fetch_doxygen
fi

# --- doxygen-awesome-css ---
DOXYGEN_AWESOME_DIR="doc/doxygen-awesome"
DOXYGEN_AWESOME_VERSION_FILE="${DOXYGEN_AWESOME_DIR}/.version"

cached_doxygen_awesome_version() {
    [[ -f "${DOXYGEN_AWESOME_VERSION_FILE}" ]] && cat "${DOXYGEN_AWESOME_VERSION_FILE}" || true
}

fetch_doxygen_awesome() {
    local base="https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v${DOXYGEN_AWESOME_CSS_VERSION}"
    local files=(
        doxygen-awesome.css
        doxygen-awesome-sidebar-only.css
        doxygen-awesome-sidebar-only-darkmode-toggle.css
        doxygen-awesome-darkmode-toggle.js
        doxygen-awesome-fragment-copy-button.js
        doxygen-awesome-interactive-toc.js
        doxygen-awesome-paragraph-link.js
    )

    echo "Downloading doxygen-awesome-css ${DOXYGEN_AWESOME_CSS_VERSION}..."
    mkdir -p "${DOXYGEN_AWESOME_DIR}"
    for f in "${files[@]}"; do
        wget -q "${base}/${f}" -O "${DOXYGEN_AWESOME_DIR}/${f}"
    done
    echo "${DOXYGEN_AWESOME_CSS_VERSION}" > "${DOXYGEN_AWESOME_VERSION_FILE}"
}

if [[ "$(cached_doxygen_awesome_version)" == "${DOXYGEN_AWESOME_CSS_VERSION}" ]]; then
    echo "doxygen-awesome-css ${DOXYGEN_AWESOME_CSS_VERSION} already present, skipping."
else
    fetch_doxygen_awesome
fi

# --- generate documentation ---
echo "  [landing]"
mkdir -p "doc/output"
touch "doc/output/.nojekyll"
doxygen "doc/landing/Doxyfile"

for doxyfile in libs/*/doc/Doxyfile; do
    lib="${doxyfile%%/doc/Doxyfile}"
    lib="${lib##libs/}"
    [[ "${lib}" == "test" ]] && continue
    echo "  [${lib}]"
    mkdir -p "doc/output/${lib}"
    doxygen "${doxyfile}"
done

echo "  [cmake]"
mkdir -p "doc/output/cmake"
doxygen "cmake/doc/Doxyfile"

echo "Done: doc/output/"
