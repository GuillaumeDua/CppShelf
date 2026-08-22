# csl::typeinfo

Compile-time type and value name stringification, without RTTI.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

> **Warning**: output is compiler-dependent and not portable across GCC, Clang, and MSVC.  
> Intended for educational use, logging and debugging, not production serialization.

## Include

```cpp
#include <csl/typeinfo.hpp>
```

## type_name_v<T>

A `constexpr inline` variable template holding the name of `T` as a `std::string_view`, extracted from compiler-provided information at compile time - no RTTI, no allocation.

```cpp
static_assert(csl::typeinfo::type_name_v<int>               == "int");
static_assert(csl::typeinfo::type_name_v<float>             == "float");
static_assert(csl::typeinfo::type_name_v<std::vector<int>>  == "std::vector<int>"); // GCC/Clang approximation
```

The underlying trait is `csl::typeinfo::type_name<T>`, whose `::value` member holds the same `std::string_view`.
For the type of a value, query `decltype`:

```cpp
static_assert(csl::typeinfo::type_name_v<decltype(42)> == "int");
```

### type_name_as_v<T, CharT>

The compiler-provided sources (`__PRETTY_FUNCTION__`, `__FUNCSIG__`) are narrow by construction, and have no
wide counterpart. `type_name_as_v<T, CharT>` is `type_name_v<T>` widened into `constexpr` per-`<T, CharT>` storage:

```cpp
static_assert(csl::typeinfo::type_name_as_v<int, wchar_t>  == L"int");
static_assert(csl::typeinfo::type_name_as_v<int, char16_t> == u"int");
```

Widening is a per-character cast, valid for the basic execution character set.
`CharT = char` is a pass-through: no storage, no conversion - `type_name_as_v<T, char>` *is* `type_name_v<T>`.

### Known limitations - type_name

| Issue                | GCC                                     | Clang                     | MSVC                                                                                |
| -------------------- | --------------------------------------- | ------------------------- | ----------------------------------------------------------------------------------- |
| Local type namespace | included (`A::B::my_type`)              | omitted (`my_type`)       | -                                                                                   |
| `std::string` alias  | `std::__cxx11::basic_string<char, ...>` | `std::basic_string<char>` | `std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >` |

See [compiler explorer demo](https://godbolt.org/z/jbfqsf7hK).

---

## value_name_v<V>

A `constexpr inline` variable template holding the string representation of the non-type template argument `V` as a `std::string_view`.

```cpp
enum class color { red, green, blue };

static_assert(csl::typeinfo::value_name_v<42>          == "42");         // GCC, Clang
static_assert(csl::typeinfo::value_name_v<color::red>  == "color::red"); // GCC, Clang
```

The underlying trait is `csl::typeinfo::value_name<V>`, whose `::value` member holds the same `std::string_view`.

### value_name_as_v<V, CharT>

The widened counterpart, mirroring `type_name_as_v` - `CharT = char` likewise being a pass-through:

```cpp
static_assert(csl::typeinfo::value_name_as_v<42, wchar_t> == L"42"); // GCC, Clang
```

### Known limitations - value_name

| Issue                  | GCC / Clang    | MSVC         |
| ---------------------- | -------------- | ------------ |
| Integer representation | decimal (`42`) | hex (`0x2a`) |

Prefer [`<charconv>`](https://en.cppreference.com/w/cpp/header/charconv) `std::to_chars` for reliable numeric stringification.  
See [compiler explorer demo](https://godbolt.org/z/cf96MnKfY).

---

## Live demo

- [godbolt.org](https://godbolt.org/z/Wd4EhcYr6)

---

## Example

@include overview.cpp
