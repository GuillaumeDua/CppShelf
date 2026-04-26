# CppShelf — Claude Code context

## Project

Collection of single-header, header-only C++ libraries (personal incubator).
MIT License. Author: Guillaume Dua "Guss".

- **Repo**: https://github.com/GuillaumeDua/CppShelf
- **Standard**: C++23 (`CMAKE_CXX_STANDARD 23`, extensions OFF)
- **Build system**: CMake 3.22+, out-of-source builds only (`build/`)
- **CI**: Ubuntu + Clang, Ubuntu + GCC

## Components

| Dir | Name | Description |
|---|---|---|
| `includes/ag/` | ag | Tuple-like interface for aggregate types |
| `includes/mp/` | mp | C++ TMP (template metaprogramming) utilities |
| `includes/wf/` | wf | Generic eDSL builder / workflow pattern |
| `includes/functional/` | functional | — |
| `includes/typeinfo/` | typeinfo | — |
| `includes/ensure/` | ensure | — |

Each component is a single header: `includes/<name>/csl/<name>.hpp`.

## Key files

- `.clang-format` — project formatter config (clang-format 22). Known limitations are documented in its header comment.
- `includes/mp/csl/mp.hpp` — largest, most complex header; template-heavy TMP code
- `includes/mp/csl/clang-format_improvements.md` — tracked formatting wishlist (also used as GitHub issue body)

## Build

```bash
cmake -B build
cmake --build build
```

Tests are under `tests/`, examples under `examples/`.

## Formatting

- Formatter: **clang-format 22**
- Config: `.clang-format` at repo root
- Validate config: `clang-format --dump-config 2>&1 >/dev/null` (errors on stderr)
- Known hard limits (no `.clang-format` fix, use `// clang-format off/on`):
  - `if (binary_expr)` continuation — always aligns after `(`, BlockIndent does not apply
  - Fold expression `+ ...` pack expansion — not split from its operand
  - Semicolon on its own line in concept definitions
  - Empty lines around `#pragma region` / `#pragma endregion`

## Coding style (mp.hpp)

- Qualifier order: `constexpr static inline` (never `inline constexpr`)
- `csl_fwd(x)` macro — forward without std::forward; do not rewrite as a function
- Short struct bodies inline: `struct Foo : Base{};` and `struct Foo { using type = T; };`
- No space before braced-init: `struct Foo : Base{}` not `struct Foo : Base {}`
- Template closing `>` on its own line (BlockIndent style)
- `requires` clause always on its own line
- Binary operators (`and`, `or`, `+`, etc.) at the **start** of continuation lines
- `noexcept(...)` breaks to its own line when preceded by a long declaration

## Environment

- OS: WSL2 (Linux on Windows)
- Paths: Windows drive mounted at `/mnt/c/`
- Project root: `/mnt/c/DEV/PROJECTS/cpp/CppShelf`
