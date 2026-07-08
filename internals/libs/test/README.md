# csl::internal::test

Lightweight, minimalistic `C++17` testing utilities shared across `CppShelf` component test suites.

Not a general-purpose testing framework - prefer `Catch2` or `GoogleTest` for that.  
Automatically enabled by the build system when any component's tests are enabled.

## Headers

| Header                                  | Description                                                                 |
| --------------------------------------- | --------------------------------------------------------------------------- |
| `csl/internal/test/test.hpp`            | `csl::internal::test::failure` exception and `csl_test_expect(expr)` macro  |
| `csl/internal/test/types/semantic.hpp`  | Test types: `not_copyable`, `not_moveable`, `strong_of<T>`                  |
| `csl/internal/test/disable_warning.hpp` | Portable `CSL_DIAG_PUSH` / `CSL_DIAG_POP` / `CSL_DIAG_DISABLE(name)` macros |

## Usage

### CMake

```cmake
target_link_libraries(<your-target> PRIVATE csl::internal::test)
```

### C++

```cpp
#include <csl/internal/test/test.hpp>

auto my_test() -> void {
    csl_test_expect(1 + 1 == 2); // throws csl::internal::test::failure on false
}
```

## Details

- ℹ️ `csl::internal::test::failure` uses `std::source_location` automatically when available (C++20), falling back to `<experimental/source_location>` or a plain message otherwise.
