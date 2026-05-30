# csl::ensure

Interface-safety utilities for C++17/20/23 - centred on a configurable **strong type** implementation.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

## Include

```cpp
#include <csl/ensure.hpp>  // dispatches to cxx_17 or >= cxx_20 automatically
```

The top-level header selects the implementation based on the language standard:

| Standard   | Implementation          |
| ---------- | ----------------------- |
| >= `C++20` | `csl/cxx_20/ensure.hpp` |
| `C++17`    | `csl/cxx_17/ensure.hpp` |

---

## `strong_type<T, tag>`

`csl::ensure::strong_type<T, tag>` wraps a value of type `T` behind a distinct identity given by `tag`, preventing accidental mixing of otherwise identical types.

```cpp
using meters   = csl::ensure::strong_type<int, struct meter_tag>;
using seconds  = csl::ensure::strong_type<int, struct second_tag>;

meters  m{ 42 };
seconds s{ 10 };

// m = s;  // error: incompatible types.
// 
// In most cases, user have to define conversion,
//  for instance, an hour is 60 minutes which are 60 seconds
//  for instance, a meter is 100 centimeters or 1000 millimeters
```

### Construction and access

Construction is **explicit**; conversion to the underlying type is **implicit** (all ref-qualifications are supported).

```cpp
meters m{ 42 };

int &       ref  = m;           // implicit lvalue ref
int         val  = m.underlying();

meters copy{ m };               // copy-construct
meters moved{ std::move(m) };   // move-construct
```

`to_underlying()` is a free function alternative to the conversion operator:

```cpp
int & v = csl::ensure::to_underlying(m); // WARNING: be careful about such reference use.
```

`unwrap()` is a generic helper that strips the strong type if present, and is a no-op otherwise:

```cpp
csl::ensure::unwrap(m);    // returns int&
csl::ensure::unwrap(42);   // returns int (no-op)
```

### Comparison

All six comparison operators are forwarded to the underlying type, constrained to only participate in overload resolution when the underlying type supports them.  
`operator<=>` is also supported when the underlying type is `std::three_way_comparable`.

```cpp
meters a{ 1 }, b{ 2 };
assert(a < b);
assert(a != b);
assert(a <=> b == std::strong_ordering::less);
```

Comparison with the raw underlying type is supported directly:

```cpp
assert(a < 2);   // meters vs int
```

### Invocation

When `T` is callable, `operator()` is forwarded to it:

```cpp
using callback = csl::ensure::strong_type<std::function<void()>, struct cb_tag>;
callback cb{ [] { /* ... */ } };
cb();
```

---

## Type traits - `csl::ensure::type_traits`

| Trait                            | Description                                                     |
| -------------------------------- | --------------------------------------------------------------- |
| `is_strong_type<T>` / `_v`       | Whether `T` is a `strong_type`                                  |
| `is_strong_type_of<T, U>` / `_v` | Whether `T` is a `strong_type` with underlying type `U`         |
| `is_tagged_by<T, tag>` / `_v`    | Whether `T` is a `strong_type` using `tag`                      |
| `underlying_type<T>` / `_t`      | The underlying type of a `strong_type`                          |
| `tag_type<T>` / `_t`             | The tag type of a `strong_type`                                 |
| `unwrap_result_type<T>` / `_t`   | `underlying_type_t<T>` if `T` is a `strong_type`, otherwise `T` |

---

## Concepts - `csl::ensure::concepts`

| Concept                              | Description                                              |
| ------------------------------------ | -------------------------------------------------------- |
| `StrongType<T>`                      | `T` is a `strong_type`                                   |
| `NotStrongType<T>`                   | `T` is not a `strong_type`                               |
| `StrongTypeOf<T, U>`                 | `T` is a `strong_type` with underlying type `U`          |
| `TaggedBy<T, tag>`                   | `T` is a `strong_type` using `tag`                       |
| `Hashable<T>` / `NoThrowHashable<T>` | `T` is a `strong_type` whose underlying type is hashable |
| `EqualityComparable<T>`              | Underlying type is `std::equality_comparable`            |
| `EqualityComparableWith<T, U>`       | Underlying types are `std::equality_comparable_with`     |
| `ThreeWayComparable<T>`              | Underlying type is `std::three_way_comparable`           |
| `ThreeWayComparableWith<T, U>`       | Underlying types are `std::three_way_comparable_with`    |

---

## STL compatibility

`std::hash` is specialised automatically for any `strong_type` whose underlying type is hashable, enabling direct use in `std::unordered_map`, `std::unordered_set`, etc.:

```cpp
std::unordered_map<meters, std::string> map;
map[meters{42}] = "forty-two";
```

CPO types are also provided for use as explicit comparators/hashers in containers:

| Type                            | Description                                             |
| ------------------------------- | ------------------------------------------------------- |
| `strong_type_hasher`            | Hash functor, delegates to `std::hash<underlying_type>` |
| `strong_type_equal_to`          | Equality functor, unwraps before comparing              |
| `strong_type_compare_three_way` | Three-way comparison functor, unwraps before comparing  |

---

## Opt-in features

Enabled via preprocessor defines before including the header.

### `CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT`

Adds `operator<<` for `std::ostream`, delegating to the underlying type's stream operator:

```cpp
#define CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT
#include <ensure/csl/ensure.hpp>

std::cout << meters{ 42 } << '\n';  // prints: 42
```

### `CSL_ENSURE__ENABLE_FMT_SUPPORT`

Adds a `fmt::formatter` specialisation, delegating to the underlying type's formatter:

```cpp
#define CSL_ENSURE__ENABLE_FMT_SUPPORT
#include <ensure/csl/ensure.hpp>

fmt::print("{}\n", meters{ 42 });   // prints: 42
```

Requires `<fmt/core.h>` and `<fmt/format.h>` to be available.
