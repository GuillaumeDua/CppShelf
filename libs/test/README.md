# csl::test

Lightweight, minimalistic `C++17` testing utilities shared across `CppShelf` component test suites.

Not a general-purpose testing framework — prefer `Catch2` or `GoogleTest` for that.  
Automatically enabled by the build system when any component's tests are enabled.

## Headers

| Header                         | Description                                                                 |
| ------------------------------ | --------------------------------------------------------------------------- |
| `csl/test/test.hpp`            | `csl::test::failure` exception and `csl_test_expect(expr)` macro            |
| `csl/test/types/semantic.hpp`  | Test types: `not_copyable`, `not_moveable`, `strong_of<T>`                  |
| `csl/test/disable_warning.hpp` | Portable `CSL_DIAG_PUSH` / `CSL_DIAG_POP` / `CSL_DIAG_DISABLE(name)` macros |

## Usage

### CMake

```cmake
target_link_libraries(<your-target> PRIVATE csl::test)
```

### C++

```cpp
#include <csl/test/test.hpp>

auto my_test() -> void {
    csl_test_expect(1 + 1 == 2); // throws csl::test::failure on false
}
```

## Details

- ℹ️ `csl::test::failure` uses `std::source_location` automatically when available (C++20), falling back to `<experimental/source_location>` or a plain message otherwise.
