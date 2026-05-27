#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

if ! command -v doxygen &>/dev/null; then
    echo "error: doxygen not found" >&2
    exit 1
fi

fetch_doxygen_awesome() {
    local dir="doc/doxygen-awesome"
    local version="v2.3.4"
    local base="https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/${version}"
    local files=(
        doxygen-awesome.css
        doxygen-awesome-sidebar-only.css
        doxygen-awesome-sidebar-only-darkmode-toggle.css
        doxygen-awesome-darkmode-toggle.js
        doxygen-awesome-fragment-copy-button.js
        doxygen-awesome-interactive-toc.js
        doxygen-awesome-paragraph-link.js
    )

    [[ -f "${dir}/doxygen-awesome.css" ]] && return

    echo "Downloading doxygen-awesome-css ${version}..."
    mkdir -p "${dir}"
    for f in "${files[@]}"; do
        wget -q "${base}/${f}" -O "${dir}/${f}"
    done
}
fetch_doxygen_awesome

for doxyfile in libs/*/doc/Doxyfile; do
    lib="${doxyfile%%/doc/Doxyfile}"
    lib="${lib##libs/}"
    [[ "${lib}" == "test" ]] && continue
    echo "  [${lib}]"
    doxygen "${doxyfile}"
done

echo "  [cmake]"
doxygen "cmake/doc/Doxyfile"

echo "Done: doc/output/"
