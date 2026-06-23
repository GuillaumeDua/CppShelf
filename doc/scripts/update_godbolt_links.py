#!/usr/bin/env python3

"""
Sync numbered example files into README.md and update Godbolt short links.

For each "<!-- EXAMPLE_BEGIN: filename --> ... <!-- EXAMPLE_END: filename -->" blocks in a library's README.md:

    - Replaces the code block with the current example file content.
    - Generates a new Godbolt short link when the file's SHA-256 has changed (or when --force is given).  
    Unchanged files reuse the cached URL so the Compiler-Explorer API is not spammed needlessly.

Usage:
  python3 doc/scripts/update_godbolt_links.py [--force] [--dry-run] [libs/ag ...]

Options:
  --force               Regenerate every link regardless of cached hash.
  --dry-run             Show what would change without writing files or calling the CE API.
  --compiler-id ID      Compiler Explorer compiler ID (default: clang_trunk).
  --compiler-options O  Compiler options passed to CE (default: -std=c++23).
  --shortlink-api URL   CE shortlink API endpoint (default: https://godbolt.org/api/shortener).
  libs/...              One or more lib directories relative to the repo root.
                        Defaults to all libs/ subdirectories.
"""

import argparse
import hashlib
import json
import re
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LOG_PREFIX = f'[{Path(__file__).stem}]'
MANIFEST_PATH = REPO_ROOT / 'doc' / '.godbolt-manifest.json'

CE_SHORTLINK_API = 'https://godbolt.org/api/shortener'
CE_COMPILER_ID = 'clang_trunk'
CE_COMPILER_OPTIONS = '-std=c++23 -O2'

# Maps each local CSL include to its raw GitHub URL counterpart.
#   CE frontend supports URL-based #include, but the API itself does not
#   so the script transforms includes pp-directives before building the session payload.
INCLUDE_URL_MAP: dict[str, str] = {
    'csl/ag.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/ag/includes/ag/csl/ag.hpp'
    ),
    'csl/mp.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/mp/includes/mp/csl/mp.hpp'
    ),
    'csl/ensure.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/ensure/includes/ensure/csl/ensure.hpp'
    ),
    'csl/functional.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/functional/includes/functional/csl/functional.hpp'
    ),
    'csl/typeinfo.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/typeinfo/includes/typeinfo/csl/typeinfo.hpp'
    ),
    'csl/wf.hpp': (
        'https://raw.githubusercontent.com/GuillaumeDua/CppShelf'
        '/refs/heads/main/libs/wf/includes/wf/csl/wf.hpp'
    ),
}

EXAMPLE_BLOCK_RE = re.compile(
    r'<!-- EXAMPLE_BEGIN: (?P<file>[^\s>]+) -->\n'
    r'(?P<body>.*?)'
    r'<!-- EXAMPLE_END: (?P=file) -->',
    re.DOTALL,
)

def sha256_of(text: str) -> str:
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def transform_includes(source: str) -> str:
    for local, url in INCLUDE_URL_MAP.items():
        source = source.replace(f'#include <{local}>', f'#include <{url}>')
    return source


def create_godbolt_link(source: str, *, compiler_id: str, compiler_options: str, api_url: str) -> str:
    payload = json.dumps({
        'sessions': [{
            'id': 1,
            'language': 'c++',
            'source': source,
            'compilers': [{
                'id': compiler_id,
                'options': compiler_options,
                'libs': [],
                'filters': {
                    'commentOnly': True,
                    'trim': True,
                    'intel': True,
                },
            }],
        }],
    }).encode('utf-8')

    req = urllib.request.Request(
        api_url,
        data=payload,
        headers={
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'User-Agent': 'CppShelf-doc-tools/1.0',
        },
        method='POST',
    )
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            result = json.loads(resp.read().decode('utf-8'))
        url = result.get('url') or f"https://godbolt.org/z/{result['id']}"
        return url
    except (urllib.error.URLError, KeyError, json.JSONDecodeError) as exc:
        raise RuntimeError(f'CE API request failed: {exc}') from exc


def build_block(source: str, url: str) -> str:
    return f'```cpp\n{source}```\n\n[![CE][ce-icon] Try me on compiler-explorer]({url})\n'


def process_lib(
    lib_dir: Path,
    manifest: dict,
    *,
    force: bool,
    dry_run: bool,
    compiler_id: str,
    compiler_options: str,
    api_url: str,
) -> int:
    readme_path = lib_dir / 'README.md'
    examples_dir = lib_dir / 'examples'
    if not readme_path.is_file() or not examples_dir.is_dir():
        return 0

    readme = readme_path.read_text(encoding='utf-8')
    matches = list(EXAMPLE_BLOCK_RE.finditer(readme))
    if not matches:
        return 0

    updated = 0
    # Iterate in reverse so earlier string offsets stay valid after substitution.
    for match in reversed(matches):
        filename = match.group('file')
        example_path = examples_dir / filename
        if not example_path.is_file():
            print(f'{LOG_PREFIX} WARNING: {example_path.relative_to(REPO_ROOT)} not found', file=sys.stderr)
            continue

        source = example_path.read_text(encoding='utf-8')
        file_hash = sha256_of(source)
        manifest_key = str(example_path.relative_to(REPO_ROOT))
        entry = manifest.get(manifest_key, {})

        if not force and file_hash == entry.get('sha256') and entry.get('url'):
            url = entry['url']
        else:
            if dry_run:
                url = entry.get('url') or 'https://godbolt.org/z/PENDING'
                print(f'{LOG_PREFIX} [dry-run] would regenerate link for {filename}')
            else:
                print(f'{LOG_PREFIX} Generating link for {filename} ...', end=' ', flush=True)
                url = create_godbolt_link(
                    transform_includes(source),
                    compiler_id=compiler_id,
                    compiler_options=compiler_options,
                    api_url=api_url,
                )
                manifest[manifest_key] = {'sha256': file_hash, 'url': url}
                print(url)
                time.sleep(0.5)

        new_section = (
            f'<!-- EXAMPLE_BEGIN: {filename} -->\n'
            f'{build_block(source, url)}'
            f'<!-- EXAMPLE_END: {filename} -->'
        )
        if new_section != match.group(0):
            readme = readme[: match.start()] + new_section + readme[match.end():]
            updated += 1

    if updated and not dry_run:
        readme_path.write_text(readme, encoding='utf-8')

    return updated


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('--force', action='store_true', help='Regenerate all Godbolt links')
    parser.add_argument('--dry-run', action='store_true', help='Preview without writing')
    parser.add_argument('--compiler-id', default=CE_COMPILER_ID, help=f'CE compiler ID (default: {CE_COMPILER_ID})')
    parser.add_argument('--compiler-options', default=CE_COMPILER_OPTIONS, help=f'CE compiler options (default: {CE_COMPILER_OPTIONS})')
    parser.add_argument('--shortlink-api', default=CE_SHORTLINK_API, help=f'CE shortlink API URL (default: {CE_SHORTLINK_API})')
    parser.add_argument(
        'libs',
        nargs='*',
        metavar='lib',
        help='Lib dirs relative to repo root (default: all under libs/)',
    )
    args = parser.parse_args()

    manifest: dict = {}
    if MANIFEST_PATH.is_file():
        manifest = json.loads(MANIFEST_PATH.read_text(encoding='utf-8'))

    lib_dirs = (
        [REPO_ROOT / p for p in args.libs]
        if args.libs
        else sorted((REPO_ROOT / 'libs').iterdir())
    )

    total = 0
    for lib_dir in lib_dirs:
        if not lib_dir.is_dir():
            continue
        print(f'{LOG_PREFIX} {lib_dir.name}:')
        n = process_lib(
            lib_dir, manifest,
            force=args.force,
            dry_run=args.dry_run,
            compiler_id=args.compiler_id,
            compiler_options=args.compiler_options,
            api_url=args.shortlink_api,
        )
        print(f'{LOG_PREFIX}   {n} example(s) updated' if n else f'{LOG_PREFIX}   nothing to update')
        total += n

    if total and not args.dry_run:
        MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
        print(f'\n{LOG_PREFIX} Manifest saved to {MANIFEST_PATH.relative_to(REPO_ROOT)}')

    return 0


if __name__ == '__main__':
    sys.exit(main())
