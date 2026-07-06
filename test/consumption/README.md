# csl: consumption tests

Standalone projects that mimic an external consumer of `csl`. They are intentionally
independent from the top-level CppShelf build - they must never be added as a subdirectory.

Two consumption modes are exercised, sharing a single [`main.cpp`](main.cpp):

| Mode | Directory | What it validates |
| ---- | --------- | ----------------- |
| FetchContent | [`fetchcontent/`](fetchcontent/) | Consuming `csl` via CMake `FetchContent` against a branch/tag |
| Install + `find_package` | [`find_package/`](find_package/) | Installing `csl` into a directory and consuming *that directory* via `find_package(csl)` |

Both run in CI on every commit via `.github/workflows/csl-consumption-ci.yml`.

## FetchContent mode

Fetches `csl` from GitHub, links its components, and compiles C++23 code against them.

```bash
bash fetchcontent/build.sh <build-dir> [extra cmake args...]
```

Cache variables:

| Variable                              | Default                                    | Description                    |
| ------------------------------------- | ------------------------------------------ | ------------------------------ |
| `csl_consumption_test_GIT_REPOSITORY` | `https://github.com/GuillaumeDua/CppShelf` | Repository to fetch `csl` from |
| `csl_consumption_test_GIT_TAG`        | current git branch, or `main`              | Branch, tag, or SHA to fetch   |

## Install + `find_package` mode

Self-contained: `build.sh` configures `csl`, installs it into a dedicated directory, then
configures the consumer with `CMAKE_PREFIX_PATH` pointing at that installed tree - so
`find_package(csl ...)` resolves **only** against the installed package, never the source tree.

```bash
bash find_package/build.sh <build-dir> [extra cmake args for the consumer...]
```

Environment:

| Variable         | Default                     | Description                              |
| ---------------- | --------------------------- | ---------------------------------------- |
| `CSL_SOURCE_DIR` | repo root (three levels up) | `csl` source tree to configure + install |

The consumer links every component as `csl::<name>` and also checks that a component-less
`find_package(csl)` provides the aggregate `csl::csl` target.

## Layout produced under `<build-dir>` (find_package mode)

```text
<build-dir>/csl-build     # csl configured from source
<build-dir>/csl-install   # csl installed here (include/csl, lib/cmake/csl)
<build-dir>/consumer      # the consumer, built against csl-install only
```
