# Overall presentation

The goal of `csl::ag` is to offer convenient ways to manipulate aggregate types.

## Overview demo

The following example demonstrates some of the features which are available in `csl::ag`.

<table>
    <tr><th>
        C++ code (
        <a href="https://godbolt.org/z/x1dGTWddK">
        Try me on compiler-explorer
        <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
        )
    </th><th> Console output </th></tr>
    <tr><td>

```cpp
struct S { char c; int i; };

static_assert(
    csl::ag::concepts::aggregate<S> and
    csl::ag::size_v<S> == 2
);
static_assert(std::same_as<char,  csl::ag::element_t<0, S>>);
static_assert(std::same_as<int,   csl::ag::element_t<1, S>>);

S value{ 'A', 41 }; ++std::get<1>(value);

using namespace csl::ag::io;
std::cout << "value: " << value << '\n';
// (wip) compatibility with `fmt` and `std::print` will be available soon
```

</td><td>

```
value: S& : {
   [0] char : A
   [1] int : 42
}
```
</td></tr></table>

## Introduction

By default, the C++ standard allow structured-binding for aggregate types.

<table>
    <tr><th>
        C++ code (
        <a href="https://godbolt.org/z/3EcK9Wc7h">
        Try me on compiler-explorer
        <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
        )
    </th></tr>
    <tr><td>

```cpp
struct type{ int i; char c; };
auto value = type{ 42, 'A' }; // NOLINT

[[maybe_unused]] auto && [ v0, v1 ] = value;
assert(v0 == 42);   // pass
assert(v1 == 'A');  // pass
```

</td></tr></table>

However, there is no - *simple* - way to access the following informations for a given aggregate type or value :

- The count of fields
- Access a field's value by index
- Iterate over fields

This library provides a way to obtain such information, and internally use it to provide convenient high-level conversions and printing functions.

---

This library is divided in five distinct parts :

- [#1](#aggregate-related-concepts) Aggregates-related concepts
- [#2](#aggregate-related-type-traits) Aggregates-related type-traits
- [#3](#to-tuple-conversion-for-aggregate-types) Conversion to tuples for aggregate types *(owning or not)*
- [#4](#tuplelike-interface-for-aggregates) A tuplelike interface for aggregates types
- [#5 (WIP)](#pretty-printing) Pretty-printing (using `std::ostream & operator<<` overloads or `fmt`)

---

## Philosophy & design choices

The key idea of this library is to ease iterations over aggregates's member-variables,  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size<T>` gives the fields count in a given aggregate type type  
  *(or [std::tuple_size_v](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size) after a `as_tuple` or `as_tuple_view` conversion)*
- `csl::ag::get<size_t N>(aggregate auto value)` allows per-field access, in a similar way to [std::get<N>](https://en.cppreference.com/w/cpp/utility/tuple/get) for [std::tuple<Ts...>](https://en.cppreference.com/w/cpp/utility/tuple)

---

## Getting starting

This library is single-header, header-only. Users may use it in various ways, however [CMake](https://cmake.org/) is the promoted one for both download and configuration.

### Integration

#### Plain download

- **Fetch** [the header file](https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/ag.hpp) and deal with the build yourself, or ...
- **Clone** the repo, or add it as a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules) to your project.

> ⚠️ Proceeding the ways enumerated above is fast & simple. 
> However this prevent users from using certain configuration mechanismes. 
> *See the [configuration](#configuration) section for more information*.

#### CMake

- **Fetch** the header file using [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html), or [ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html).

Then use the `csl::ag` target.

> Note : to disable tests, set the cmake cache variable `CSL_BUILD_ALL_TESTS` to false.

### Configuration

#### Bitfields support

⚠️ By default, bitfields support is **disabled**.  
Using features for this library with any aggregate type using custom layout will results in ☣️ **undefined behavior**.  
Most likely, a compile-time error will be emitted. However, such behavior is not guaranteed.

```cpp
struct S {
    int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
    char : 0;
    char && c;
};
static_assert(csl::ag::size_v<S> == 5); // ☣️ UB by default
```

If you plan to use features of this library with aggregate types containing bitfields, you must first enable such support either using one of the two following ways :

- Using `CMake`, edit the cache to set the `CSL_AG_ENABLE_BITFIELDS_SUPPORT` option to `on`.
- Using plain **C++**, define the preprocessor variable `CSL_AG_ENABLE_BITFIELDS_SUPPORT`.
  ```cpp
  #define CSL_AG_ENABLE_BITFIELDS_SUPPORT true
  ```

> ❔ **Question** : Why such option exists ?
> 
> The *(compile-time)* algorithm internally used by the library to count fields for aggregate types possibly containing bitfields is much slower than the default one.  
> One might want to challenge his/her project's design in order to avoid such high performance cost.

#### Highier limit for aggregate field count

This library relies on a **CMake** cache variable `CSL_AG_MAX_FIELDS_COUNT_OPTION` to generate code in order to properly handle aggregate types with fields up to this value *(default : `128`)*.

The sources by default offer support for aggregate types up to `CSL_AG_MAX_FIELDS_COUNT_OPTION`, meaning 128 fields.

To extend such support, simply edit your **CMake** cache to set a greater integral value.

---

## Content

### Aggregate-related concepts

All concepts that are part of the public interface are defined in the namespace `csl::ag::concepts`.

#### unqualified_aggregate<T>

Requirements that given `T` type must meet to be considered as an unqualified (e.g, not cvref-qualified) aggregate type by this library components.

- `std::is_aggregate_v<T>`
- `not std::is_empty_v<T>`
- `not std::is_union_v<T>`
- `not std::is_polymorphic_v<T>`
- `not std::is_reference_v<T>`

More requirements are likely to be added, in order to handle specific layout *(bitset, custom aligments, etc.)*.

#### aggregate<T>

`T` must be a possibly cvref-qualified aggregate, meeting the `unqualified_aggregate<std::remove_cvref_t<T>>` requirement.

Note that such requirement is widely used in this library.

#### aggregate_constructible_from<T, args_ts...>

`T` must be a valid aggregate type, constructible using brace-initialization using values of types `args_ts...`.

#### aggregate_constructible_from_n_values<T, std::size_t N>

`T` must be a valid aggregate type, constructible using `N` values (which types does not matter here).  
This does not mean that `T` has `N` fields : it can be more.

#### tuplelike<T>

`T` must the tuplelike interface, with valid implementation of :

- [std::tuple_size_v](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size)
- [std::get<std::size_t>(/*possibly cvref-qualified */ T)](https://en.cppreference.com/w/cpp/utility/tuple/get)
- [std::tuple_element<std::size_t, T>](https://en.cppreference.com/w/cpp/utility/tuple/tuple_element)

#### csl::ag::concepts::structured_bindable<T>

`T` must either match `tuplelike<T>` or `aggregate<T>` requirements.

See the [structured_binding documentation](https://en.cppreference.com/w/cpp/language/structured_binding) for more details.

### Aggregate-related type-traits

#### csl::ag::size<T>

Integral constant type which value represents the count of fields for a given aggregate type.

```cpp
struct A{ int i; float f; };
static_assert(csl::ag::size_v<A> == 2);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/5cr1x7K3T).

Just like `std::tuple_size/std::tuple_size_v`, the **value** can be accessed using a convenience alias :

```cpp
template <csl::ag::concepts::aggregate T>
constexpr auto size_v = size<T>::value;
```

#### csl::ag::element<std::size_t, T>

Type-identity for a field's type of a given aggregate type.

```cpp
struct A{ int i; float f; };
static_assert(std::same_as<int,   csl::ag::element_t<0, A>>);
static_assert(std::same_as<float, csl::ag::element_t<1, A>>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/xMYzezxoo).

Just like `std::tuple_element/std::tuple_element_t`, the **type** can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using element_t = typename element<N, T>::type;
```

#### csl::ag::view_element

In a similar way to `csl::ag::element<std::size_t, T>`, `csl::ag::view_element<std::size_t,T>` is a type-identity for a field's type of a given aggregate view type.  
For more details about aggregate's view, see the [to-tuple non-owning conversion (view)](#non-owning-conversion-view) section.

```cpp
struct A{ int i; float & f; const char && c; };

static_assert(std::same_as<int&&,        csl::ag::view_element_t<0, A&&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, A&&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, A&&>>);

static_assert(std::same_as<const int&,    csl::ag::view_element_t<0, const A&>>);
static_assert(std::same_as<float&,        csl::ag::view_element_t<1, const A&>>);
static_assert(std::same_as<const char&&,  csl::ag::view_element_t<2, const A&>>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/xMYzezxoo).

The `type` nested-type can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using view_element_t = typename view_element<N, T>::type;
```

### to-tuple conversion for aggregate types

This library provides two ways to convert an aggregate's value to [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple), distinguishing between proprietary and non-proprietary tuples of values.

- **Owning** is a plain translation of an aggregate type as a tuple.

  Each `std::tuple_element_t` of the resulting type will be strictly equivalent to `csl::ag::element_t` of the source one.  
  The value of each field is pass by-value *(understand: copy)*.

  See the [Owning conversion](#owning-conversion) section hereunder.

- **Non-owning** *(undestand: **view**, or accessor)* conversion offer a cheap way to convert an aggregate into a tuple of references;  
  offering a convenient way to then use already-existing features - *or even libraries* - that operates on [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple) values.

  - Field types that already are references will remain unchanged : `csl::ag::element_t` is strictly equivalent to `std::tuple_element_t`.
  - Field types that are not references will acquire the cvref-qualifier of the source aggregate value.

  See the [Non-owning conversion (view)](#non-owning-conversion-view) section hereunder.

#### Owning conversion

In opposition to the previous conversion that creates non-owning views,  
the following create copies of each members, discarding cvref-qualifiers.

```cpp
struct A{ int i; float f; };
constexpr auto value = csl::ag::to_tuple(A{ .i = 42, .f = 0.13f });

[&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    ((std::cout << std::get<indexes>(value) << ' '), ...);
}(std::make_index_sequence<csl::ag::size_v<A>>{});

static_assert(std::same_as<
    int,
    std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
>);
static_assert(std::same_as<
    float,
    std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/Yv6WKssG4).

The main advantage here is to use such function in `constexpr` contexts.
A precondition here is that each aggregates field's value must be usable in a constexpr context (e.g not ref-qualified).

```cpp
struct A{ int & i; float && f; };
int i = 42; float f = .13f;
/* not constexpr */ auto value = csl::ag::to_tuple(A{ .i = i, .f = std::move(f) });
// ^^^^^^^^^^^^^^^ : neither `A::i` nor `A::f` are valid constants

[&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    ((std::cout << std::get<indexes>(value) << ' '), ...);
}(std::make_index_sequence<csl::ag::size_v<A>>{});

static_assert(std::same_as<
    int,
    std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
>);
static_assert(std::same_as<
    float,
    std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/K4qzsxcGY).

#### Non-owning conversion (view)

This function returns a non-owning tuple (`std::tuple` of references), for which each element represents a given aggregate's field.

Note that the reference semantic of the aggregate's value is used to qualify each tuple's elements.

```cpp
struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };
int i = 42;

{ // using a rvalue
  [[maybe_unused]] auto as_tuple = csl::ag::as_tuple_view(type{ i, i, i, std::move(i) }); // not constexpr yet
  static_assert(std::same_as<
      decltype(as_tuple),
      std::tuple<int&&, int&&, const int&&, int&&>
  >);
}
{ // using a const-lvalue
  const auto & value = type{ i, i, i, std::move(i) };
  [[maybe_unused]] auto as_tuple = csl::ag::as_tuple_view(value); // not constexpr yet
  static_assert(std::same_as<
      decltype(as_tuple),
      std::tuple<const int &, int&, const int &, int&>
  >);
}
```

Additionally, `csl::ag::view_element(_t)<N,T>` can be used to obtains a field type information.

```cpp
static_assert(std::same_as<
    int &&,
    csl::ag::view_element_t<0, decltype(value)>
>);
static_assert(std::same_as<
    char &&,
    csl::ag::view_element_t<1, std::remove_cvref_t<decltype(value)>>
>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/z8vnxr619).


### tuplelike interface for aggregates

#### std::tuple_element

```cpp
struct type{ const int i = 0; char & c; };
char c = 'c';
auto value = type{ 42, c }; // NOLINT

static_assert(std::same_as<
    const int,
    std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
>);
static_assert(std::same_as<
    char&,
    std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/YPj7931b9).

#### std::get

Simple example :

```cpp
struct A{ int i; float f; };
auto value = A { .i = 42, .f = 0.13f };

std::cout << std::get<0>(value) << ", " << std::get<1>(value) << '\n';

static_assert(std::same_as<
    int &,
    decltype(std::get<0>(value))
>);
static_assert(std::same_as<
    float &,
    decltype(std::get<1>(value))
>);
```

```
42, 0.13
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/je4Gr16h5).

Slightly more advanced example :

```cpp
struct A{ int i; float f; };
auto value = A{ .i = 42, .f = 0.13f };

[&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    ((std::cout << std::get<indexes>(value) << ' '), ...);
}(std::make_index_sequence<csl::ag::size_v<A>>{});
```

```
42 0.13 
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/j9bhr4WrP).

Note that `constexpr`-ness is preserved :

```cpp
struct A{ int i; char c; };
constexpr auto value = A{ 42, 'c' };
static_assert(csl::ag::get<0>(value) == 42);    // pass
static_assert(csl::ag::get<1>(value) == 'c');   // pass
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/h9jbrc8d6).

### Pretty-printing

There are two way to pretty-print aggregate types :

- using the legacy C++'s way : `std::ostream& operator<<(std::ostream&, T&&)` overload
- using the `fmt` or `std::format` library

#### using std::ostream :

Simple example :

```cpp
#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
  using namespace csl::ag::io;
  
  struct A{ int i; float f; };
  std::cout << A{ .i = 42, .f = .13f };
}
```

```
A && : {
   [0] int : 42
   [1] float : 0.13
}
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/q8Yeq4e83).

Advanced example :

```cpp
#include <iostream>
#include <tuple>
#include <array>

struct A{ int i; float f; };
struct B{};
auto & operator<<(std::ostream & os, B) { 
  return os << "user-defined operator<<(std::ostream&, const B &)";
}
struct C {
    A a;
    B b;
    int & i;
    const std::string str;
    char && c;
    std::tuple<bool, int> t{ true, 2 };
    std::array<char, 3> arr{ 'a', 'b', 'c' };
};

#include <csl/ag.hpp>

auto main() -> int {
  using namespace csl::ag::io;

  int i = 42;
  char c = 'c';
  auto value = C { 
    .a = A{ 13, .12f },
    .b = B{},
    .i = i, .str = "str", .c = std::move(c)
  };
  std::cout << value;
}
```

Output :

```yaml
C & : {
   [0] A & : {
      [0] int : 13
      [1] float : 0.12
   }
   [1] B & : user-defined operator<<(std::ostream&, const B &)
   [2] int & : 42
   [3] const std::basic_string<char> : str
   [4] char && : c
   [5] std::tuple<bool, int> & : {
      [0] bool : 1
      [1] int : 2
   }
   [6] std::array<char, 3> & : {
      [0] char : a
      [1] char : b
      [2] char : c
   }
}
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/hsofqExoT).

## std::tuple and aggregate types homogeneity

As is, it is quite easy to handle aggregates and tuple in an homogeneous way, despite limitation listed in the next section below.

```cpp
void do_stuff_with_either_a_tuple_or_aggregate(csl::ag::concepts::structured_bindable auto && value) {

    using value_type = std::remove_cvref_t<decltype(value)>;

    constexpr auto size = []() constexpr { // work-around for ADL issue
        if constexpr (csl::ag::concepts::tuplelike<value_type>)
            return std::tuple_size_v<value_type>;
        else if constexpr (csl::ag::concepts::aggregate<value_type>)
            return csl::ag::size_v<value_type>;
        else
            static_assert(sizeof(value_type) and false, "Unexpected type"); // NOLINT
    }();

  const auto do_stuffs = [&]<size_t index>(){
    auto && element_value = std::get<index>(std::forward<decltype(value)>(value));
    using element_value_type = decltype(element_value);
    using element_type = std::tuple_element_t<index, value_type>;

    // do stuffs with element_value, element_type ...
  };

  [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
      ((do_stuffs.template operator()<indexes>()), ...);  
  }(std::make_index_sequence<size>{});
}
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/j6ahehMn1).

## Current limitations

As-is, this implementation internally relies on structured-binding, which design choice expose two main limitations :

- Compile-time evaluation is limited.
- By-default behaviors injections, using STL extension/customization point (e.g injecting in the `std` namespace definitions for `get`/`tuple_element`/`tuple_size(_v)` won't work).
- Aggregate types with more fields than their size are currently not supported.
- Ill-formed aggregate types using union-fields are not supported

## (Internal details) Where's the magic ?

Everything has its own dirty secrets, and this library is no exception.  

Internally, and for each given aggregate type, it recursively check if a value of the later is constructible from an aggregate-initialization using `N` implicitly-castable-to-anything parameters values.  

> The initial `N` value is `sizeof(T)`.

If the result is a failure, then another attempt using `N-1` is done, up to 1 (included).

See `csl::ag::concepts::aggregate_with_n_fields<T, size>`

```cpp
auto main() -> int {
    struct A{ char a, b, c, d, e, f, g, h; };
    static_assert(sizeof(A) == 8);
    static_assert(csl::ag::size_v<A> == 8);

    struct B{ int a, b; };
    static_assert(sizeof(B) == 8);
    static_assert(csl::ag::size_v<B> == 2);

    struct alignas(32) C { char c; };
    static_assert(sizeof(C) == 32);
    static_assert(csl::ag::size_v<C> == 1);
}
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/v91bqTEWP).
