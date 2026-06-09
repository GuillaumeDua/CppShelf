# csl cmake modules

Reusable CMake utilities provided by [CppShelf](https://github.com/GuillaumeDua/CppShelf).

---

## `csl/cache_entry_conversion`

```cmake
include(csl/cache_entry_conversion)
csl_cache_entry_to_property_index(ID <cache-var> OUT_VAR <output-var>)
```

Converts a `CACHE STRING` entry to the integer index of its current value within the entry's `STRINGS` property. The cache entry must have its allowed values registered via `set_property(CACHE <var> PROPERTY STRINGS ...)` beforehand.

Fails with a fatal error if the cache entry has no `STRINGS` property or if the current value is not among the allowed values.

---

## `csl/get_cpm`

```cmake
include(csl/get_cpm)
csl_get_cpm([VERSION <version>])
```

Downloads [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) and includes it.
Retries up to 5 times on transient network failures.
Respects `CPM_SOURCE_CACHE` and `ENV{CPM_SOURCE_CACHE}` for the download location; falls back to `${CMAKE_BINARY_DIR}/cmake/`.

Cache entries:

| Variable                      | Default  | Description                                          |
| ----------------------------- | -------- | ---------------------------------------------------- |
| `CSL_GET_CPM_DEFAULT_VERSION` | `0.42.1` | CPM.cmake version used when `VERSION` is not passed. Can be overridden via `-DCSL_GET_CPM_DEFAULT_VERSION=<ver>` |

---

## `csl/graphviz`

```cmake
include(csl/graphviz)
csl_graphviz(
    TARGET       <target-name>
    OPTIONS_FILE <path/to/CMakeGraphVizOptions.cmake>
    [OUTPUT_DIR  <dir>]       # default: <source_dir>/doc/images
    [FORMAT      svg|png|...] # default: svg; skipped if dot is not found
)
```

Creates a custom target `csl_graphviz_<target>` that:

1. Copies `OPTIONS_FILE` into `CMAKE_BINARY_DIR` as `CMakeGraphVizOptions.cmake`.
2. Runs `cmake --graphviz` to produce `<target>.dot`.
3. If `dot` (Graphviz) is found, renders it to `<target>.<format>`.

`TARGET` must be an existing CMake target. If `dot` is not found, the image rendering step is skipped with a warning and only the `.dot` file is produced.

---

## `csl/list_subdirectories`

```cmake
include(csl/list_subdirectories)
csl_list_subdirectories(DIRECTORY <dir> RESULT <output-var>)
```

Macro that populates `<output-var>` with a list of all subdirectories directly under `<dir>`.

---

## `csl/print_aligned`

```cmake
include(csl/print_aligned)
```

Provides aligned-column status output for CMake configure-time logging.

Cache entries:

| Variable                                             | Default | Description                                   |
| ---------------------------------------------------- | ------- | --------------------------------------------- |
| `CSL_PRINT_ALIGNED_DEFAULT_WIDTH`                    | `50`    | Default column width                          |
| `CSL_PRINT_ALIGNED_ACCOUNT_FOR_CMAKE_MESSAGE_INDENT` | `ON`    | Account for `CMAKE_MESSAGE_INDENT` in padding |

### `csl_print_aligned`

```cmake
csl_print_aligned(<log_level> <variable> [width <n>] [filler_char <c>] [depends <other-var>])
```

Prints `[<project>] <variable> .... <value>` at the given log level, padded to the configured column width. The optional `depends` argument suppresses output when `<variable>` and `<other-var>` have the same boolean value.

### `csl_check_option`

```cmake
csl_check_option(<option_name>)
```

Prints a `CHECK_START` / `CHECK_PASS` / `CHECK_FAIL` message for a boolean cache option.

### `csl_print_build_info`

```cmake
csl_print_build_info()
```

Prints a standard block of build-relevant CMake variables (compiler, standard, paths, generator).
