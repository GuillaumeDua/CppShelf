# csl: integration test

Standalone project that mimics an external consumer of `csl`, using CMake FetchContent.  
It is intentionally independent from the top-level CppShelf build - it must never be added as a subdirectory.

## Purpose

Validates that any user can consume `csl` as a third-party library via `CMake` `FetchContent`,
link against its components, and compile C++23 code against them.

Runs in CI on every commit via `.github/workflows/csl-integration-ci.yml`.

## Build

### Using `build.sh`

Configures, builds, and runs `ctest` in one step:

```bash
bash build.sh <build-dir> [extra cmake args...]
```

Example:

```bash
bash build.sh /tmp/csl-integration-build
```

### Using `CMake` directly

```bash
cmake -B <build-dir> -S . -G Ninja
cmake --build <build-dir>
ctest --test-dir <build-dir> --output-on-failure
```

## Cache variables

| Variable                              | Default                                    | Description                    |
| ------------------------------------- | ------------------------------------------ | ------------------------------ |
| `csl_integration_test_GIT_REPOSITORY` | `https://github.com/GuillaumeDua/CppShelf` | Repository to fetch `csl` from |
| `csl_integration_test_GIT_TAG`        | current git branch, or `main`              | Branch, tag, or SHA to fetch   |
