# csl::typeinfo

Compile-time type and value name stringification, without RTTI.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

> **Warning**: output is compiler-dependent and not portable across GCC, Clang, and MSVC.  
> Intended for educational use, logging and debugging, not production serialization.

## Include

```cpp
#include <typeinfo/csl/typeinfo.hpp>
```

## type_name_v<T>

A `constexpr inline` variable template holding the name of `T` as a `std::string_view`, extracted from compiler-provided information at compile time - no RTTI, no allocation.

```cpp
static_assert(csl::typeinfo::type_name_v<int>               == "int");
static_assert(csl::typeinfo::type_name_v<float>             == "float");
static_assert(csl::typeinfo::type_name_v<std::vector<int>>  == "std::vector<int>"); // GCC/Clang approximation
```

A function form is also available for contexts where the variable template cannot be used:

```cpp
std::string_view name = csl::typeinfo::type_name<T>();
```

`type_name` is also overloaded for non-type template parameters - `type_name<value>()` returns the type of `value`:

```cpp
static_assert(csl::typeinfo::type_name<42>() == "int");
```

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

A function form is also available:

```cpp
std::string_view name = csl::typeinfo::value_name<V>();
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
