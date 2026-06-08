
<div style="position: absolute; top: 0; right: 17px;">
  <a href="https://github.com/GuillaumeDua/CppShelf">
    <img loading="lazy" width="149" height="149" src="https://github.blog/wp-content/uploads/2008/12/forkme_right_red_aa0000.png?resize=149%2C149" class="attachment-full size-full" alt="Fork me on GitHub" data-recalc-dims="1" align="right"
    style="position: relative; top: 0; right: 0; z-index: 1;">
  </a>
  <a href="https://guillaumedua.github.io/CppShelf/">
    <img loading="lazy" src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/gh-pages-logo.png?raw=true" alt="Check documentation on GitHub-pages" align="right" class="attachment-full size-full" height="40"
    style="position: absolute; top: 0; right: 0; z-index: 3;"
    >
  </a>
</div>

# Overall presentation

The goal of `csl::ag` is to offer convenient ways to manipulate aggregate types.

## Overview demo

The following example demonstrates some of the features which are available in `csl::ag`.

<table>
    <tr><th>
        C++ code (
        <a href="https://godbolt.org/z/x1dGTWddK">
        Try me on compiler-explorer
        <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
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
```

</td><td>

```text
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
        <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
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

This library is divided in six distinct parts :

- <a href="#concepts">#1 - Concepts</a>
- <a href="#type-traits">#2 - Type-traits</a>
- <a href="#to-tuple-conversion">#3 - to-tuple conversion (owning or non-owning)</a>
- <a href="#tuplelike-interface">#4 - tuplelike interface</a>
- <a href="#functional-api">#5 - Functional API (`apply`, `for_each`)</a>
- <a href="#formatting-and-printing">#6 - Formatting and printing</a>

---

## Philosophy & design choices

The key idea of this library is to ease iterations over aggregates's member-variables,  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size<T>` gives the fields count in a given aggregate type type  
  (or [std::tuple_size_v](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size) after a `to_tuple` or `to_tuple_view` conversion)
- `csl::ag::get<size_t N>(aggregate auto && value)` allows per-field access, in a similar way to [std::get<N>](https://en.cppreference.com/w/cpp/utility/tuple/get) for [std::tuple<Ts...>](https://en.cppreference.com/w/cpp/utility/tuple)

---

## Getting starting

This library is single-header, header-only. Users may use it in various ways, however [CMake](https://cmake.org/) is the promoted one for both download and configuration.

### Integration

#### Plain download

- **Fetch** [the header file](https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/libs/ag/includes/ag/csl/ag.hpp) and deal with the build yourself, or ...
- **Clone** the repo, or add it as a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules) to your project.

> ⚠️ Proceeding the ways enumerated above is fast & simple. 
> However this prevent users from using certain configuration mechanismes. 
> *See the [configuration](#configuration) section for more information*.

#### CMake

- **Fetch** the header file using [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html), or [ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html).

Then use the `csl::ag` target.

> Note : to disable tests, set the cmake cache variable `CSL_BUILD_ALL_TESTS` to false.

### Configuration

This project can be configured using the following cmake cache entries, grouped by categories:

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
                                        // 💡 well-defined with CSL_AG__ENABLE_BITFIELDS_SUPPORT=ON
```

If you plan to use features of this library with aggregate types containing bitfields, you must first enable such support either using one of the two following ways :

- Using `CMake`, edit the cache to set the `CSL_AG__ENABLE_BITFIELDS_SUPPORT` option to `on`.  
  or
- Using plain **C++**, define the preprocessor variable `CSL_AG__ENABLE_BITFIELDS_SUPPORT`.

  ```cpp
  #define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
  ```

> ❔ **Question** : Why such option exists ?
>
> The (compile-time) algorithm internally used by the library to count fields for aggregate types possibly containing bitfields is much slower than the default one.  
> One might want to challenge his/her project's design in order to avoid such high performance cost.

#### Highier limit for aggregate field count

This library relies on a **CMake** cache variable `CSL_AG__MAX_SUPPORTED_FIELDS_COUNT` to generate code in order to properly handle aggregate types with fields up to this value.

By default, `CSL_AG__MAX_SUPPORTED_FIELDS_COUNT` is set to `128`, meaning the library supports aggregate types with up to 128 fields.

To extend such support, edit your **CMake** cache to set `CSL_AG__MAX_SUPPORTED_FIELDS_COUNT` to a greater integral value.

> ❔ **Question** : What if I don't use **CMake** ?
> 
> Then the library will always use the default value.

> ❔ **Question** : Why such configuration/limitation ?
>
> Despite interesting proposals that aim to enhance & offer new code generation mecanisms as part of the C++ language, such features are not available yet.  
> 
> The choice here to use **CMake** in order to generate C++ code **upstream** is a reasonable trade-off to guarantee easier debugging and avoid dark-magic tricks (such as relying on PP macros, etc.).
> 
> 👉 If you are willing to propose a better design, you can submit a [PR here](https://github.com/GuillaumeDua/CppShelf/pulls).

#### Formatting and printing (experimentale)

⚠️ This section is **experimentale**, and **SHOULD NOT** be used in production.  
Breaking changes are very likely, as the API is instable **for now**.

All options in this section are opt-ins *(`OFF` by default)*

- `CSL_AG__ENABLE_FORMAT_SUPPORT`: add `std::formatter<csl::ag::aggregate T>`

  ```cpp
  const auto formatted = std::format("my aggregate = {}", my_aggregate{});
  // formatted == "my aggregate = {'a', {13}}"
  std::print("{}", my_aggregate_value);   // default presentation (compact) → {'a', {13}}
  std::print("{:c}", my_aggregate_value); // compact presentation           → {'a', {13}}
  std::print("{:p}", my_aggregate_value); // pretty  presentation           → see below
  ```

- `CSL_AG__ENABLE_FMTLIB_SUPPORT`: makes `csl::ag` depends on the `fmt` library, and add `fmt::formatter<csl::ag::aggregate T>`.

  > If [fmtlib](https://github.com/fmtlib/fmt)'s `cmake` target `fmt::fmt-header-only` is not available when building `csl::ag` with `CSL_AG__ENABLE_FMTLIB_SUPPORT` set to `ON`, then such a dependency will be injected using `cmake FetchContent`.

  ```cpp
  const auto formatted = fmt::format("my aggregate = {}", my_aggregate{});
  // formatted == "my aggregate = {'a', {13}}"
  fmt::print("{}", my_aggregate_value);   // default presentation (compact) → {'a', {13}}
  fmt::print("{:c}", my_aggregate_value); // compact presentation           → {'a', {13}}
  fmt::print("{:p}", my_aggregate_value); // pretty  presentation           → see below
  ```

- `CSL_AG__ENABLE_IOSTREAM_SUPPORT`: add `csl::ag::io::operator<<(const csl::io::indented_ostream os, csl::ag::concepts::structured_bindable auto && value)`

  ```cpp
  using namespace csl::ag::io;
  std::cout << my_aggregate{}; // equivalent to: `indented_ostream{std::cout} << my_aggregate{};`
  indented_ostream{std::cout, 2} << my_aggregate{}; // explicit indentation
  ```

About compact vs. pretty presentations:

```cpp
struct A{ int i{}; };
struct my_aggregate{ char c = 'a'; A a = A{ .i = 13} };
```

- Compact presentation:

```text
my_aggregate: { char: 'c', A: { int: 13 } }
```

- Pretty presentation:

```text
my_aggregate: {
    char: 'c',
    A: {
        int: 13
    }
}
```

## Content

All components that are part of the public interface are defined in the namespace `csl::ag`,  
except for nested-namespaces named `details`.  

In other words, the library provides **no guarantee** to any direct use of namespaces named with a pattern like `csl::ag::*::details::*`.

### Concepts

All concepts that are part of the public interface are defined in the namespace `csl::ag::concepts`.

#### unqualified_aggregate<T>

Requirements that given `T` type must meet to be considered as an unqualified (e.g, not cvref-qualified) aggregate type by this library components.

- `std::is_aggregate_v<T>`
- `not std::is_empty_v<T>`
- `not std::is_union_v<T>`
- `not std::is_polymorphic_v<T>`
- `not std::is_reference_v<T>`

#### aggregate<T>

`T` must be a possibly cvref-qualified aggregate, meeting the `unqualified_aggregate<std::remove_cvref_t<T>>` requirement.

Note that such requirement is widely used in this library.

#### aggregate_constructible_from<T, args_ts...>

`T` must be a valid aggregate type, constructible using brace-initialization using values of types `args_ts...`.

#### aggregate_constructible_from_n_values<T, std::size_t N>

`T` must be a valid aggregate type, constructible using `N` values (which types does not matter here).  
This does not mean that `T` has `N` fields : it can be more.

#### tuplelike<T>

`T` must meet the tuplelike interface, with valid implementation of :

- [std::tuple_size_v](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size)
- [std::get<std::size_t>(/*possibly cvref-qualified */ T)](https://en.cppreference.com/w/cpp/utility/tuple/get)
- [std::tuple_element<std::size_t, T>](https://en.cppreference.com/w/cpp/utility/tuple/tuple_element)

#### csl::ag::concepts::structured_bindable<T>

`T` must either match `tuplelike<T>` or `aggregate<T>` requirements.

See the [structured_binding documentation](https://en.cppreference.com/w/cpp/language/structured_binding) for more details.

### Type-traits

#### csl::ag::size<T>

Integral constant type which value represents the count of fields for a given aggregate type.

```cpp
struct A{ int i; float f; };
static_assert(csl::ag::size<A>::value == 2);
static_assert(csl::ag::size_v<A>      == 2);
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/5cr1x7K3T).

Just like `std::tuple_size`/`std::tuple_size_v`, the **value** can be accessed using a convenience alias :

```cpp
template <csl::ag::concepts::aggregate T>
constexpr inline static auto size_v = size<T>::value;
```

#### csl::ag::element<std::size_t, concepts::aggregate>

Type-identity of a field's type of a given aggregate type.

```cpp
struct A{ int i; float f; };
static_assert(std::same_as<int,   csl::ag::element_t<0, A>>);
static_assert(std::same_as<float, csl::ag::element_t<1, A>>);
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/xMYzezxoo).

Just like `std::tuple_element/std::tuple_element_t`, the **type** can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using element_t = typename element<N, T>::type;
```

#### csl::ag::view_element<std::size_t, concepts::aggregate>

In a similar way to `csl::ag::element<std::size_t, T>`, `csl::ag::view_element<std::size_t,T>` is a type-identity for a field's type of a given aggregate view type.  
For more details about aggregate's view, see the [to-tuple non-owning conversion (view)](#non-owning-conversion-view-lightweight-accessor) section.

```cpp
struct A{ int i; float & f; const char && c; };

static_assert(std::same_as<int&&,        csl::ag::view_element_t<0, A&&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, A&&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, A&&>>);

static_assert(std::same_as<const int&,    csl::ag::view_element_t<0, const A&>>);
static_assert(std::same_as<float&,        csl::ag::view_element_t<1, const A&>>);
static_assert(std::same_as<const char&&,  csl::ag::view_element_t<2, const A&>>);
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/xMYzezxoo).

The `type` nested-type can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using view_element_t = typename view_element<N, T>::type;
```

### to-tuple conversion

This library provides two ways to convert an aggregate's value to [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple), distinguishing between proprietary and non-proprietary tuples of values.

- **Owning** is a plain translation of an aggregate type as a tuple.

  Each `std::tuple_element_t` of the resulting type will be strictly equivalent to `csl::ag::element_t` of the source one.  
  The value of each field is pass by-value (understand: copy).

  See the [Owning conversion](#owning-conversion) section hereunder.

- **Non-owning** (undestand: **view**, or **lightweight accessor**) conversion offers a cheap way to convert an aggregate into a tuple of references;  
  offering a convenient way to then use already-existing features - *or even libraries* - that operates on [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple) values.

  - Field types that already are references will remain unchanged : `csl::ag::element_t` is strictly equivalent to `std::tuple_element_t`.
  - Field types that are not references will acquire the cvref-qualifier of the source aggregate value.

  See the [Non-owning conversion (view, lightweight accessor)](#non-owning-conversion-view-lightweight-accessor) section hereunder.

#### Owning conversion

The following factory (as a function) creates a `std::tuple` of `csl::ag::elements`, never altered.  

The value of each member-variable of the aggregate's value are [forward](https://en.cppreference.com/w/cpp/utility/forward)ed, in order to preserve cvref-semantic.  
Meaning that using a `const-lvalue-reference` of a given type `S` 's value will result in a copy of each of its field that are not ref-qualified,  
while using a `rvalue-reference` will results in a perfect-forwarding that member-variable.

<table>
    <tr><th>
        C++ code (
        <a href="https://godbolt.org/z/Yqh1q3Wea">
        Try me on compiler-explorer
        <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
        )
    </th><th> Console output </th></tr>
    <tr><td>

```cpp
struct A{ int i; float f; };

constexpr auto value = A{ .i = 42, .f = 0.13f };
constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

[&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    static_assert((std::same_as<
        csl::ag::element_t<indexes, A>, // { 0: int, 1:float }
        std::tuple_element_t<indexes, std::remove_cvref_t<decltype(value_as_tuple)>>
    > and ...));

    ((std::cout << std::get<indexes>(value_as_tuple) << ' '), ...);
}(std::make_index_sequence<csl::ag::size_v<A>>{});
```

</td><td>

```text
42 0.13
```
</td></tr></table>

The main advantage here is to use such function in a `constexpr` contexts.  
A precondition while doing so is that each aggregates field's value must be usable in a constexpr context though (e.g **not** ref-qualified).

```cpp
struct A{ int i; float f; };

static_assert(std::same_as<
    std::tuple<int, float>,
    csl::ag::to_tuple_t<A>
>);

static_assert(std::same_as<int, csl::ag::element_t<0, A>>);
static_assert(std::same_as<int, std::tuple_element_t<0, csl::ag::to_tuple_t<A>>>);
static_assert(std::same_as<int, std::tuple_element_t<0,decltype(csl::ag::to_tuple(A{}))>>);

constexpr auto value = A{ .i = 42, .f = 0.13f };
constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

static_assert(42    == std::get<0>(value_as_tuple));
static_assert(0.13f == std::get<1>(value_as_tuple));
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/EE7494zbv).

Additionaly, [std::tuple_element_t](https://en.cppreference.com/w/cpp/utility/tuple/tuple_element) can be use to obtains the conversion result's element types.

- Example 1 : aggregate type with not-cvref-qualified fields

  <table><tr><th>
  C++ code (
  <a href="https://godbolt.org/z/17Es3oooY">
  Try me on compiler-explorer
  <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
  )
  </th><th> Console output </th></tr>
  <tr><td>
  
  ```cpp
  struct A{ int i; float f; };
  constexpr auto value = csl::ag::to_tuple(A{ .i = 42, .f = 0.13f });

  [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
      ((std::cout << std::get<indexes>(value) << ' '), ...);
  }(std::make_index_sequence<csl::ag::size_v<A>>{});

  static_assert(std::same_as<
      int,
      std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
  >);                      // \-> same as csl::ag::to_tuple_t<A>
  static_assert(std::same_as<
      float,
      std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
  >);
  ```
  
  </td><td>
  
  ```
  42 0.13 
  ```
  
  </td></tr></table>

- Example 2 : aggregate type with ref-qualified fields

  <table><tr><th>
  C++ code (
  <a href="https://godbolt.org/z/17Es3oooY">
  Try me on compiler-explorer
  <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
  )
  </th><th> Console output </th></tr>
  <tr><td>
  
  ```cpp
  struct A{ int & i; float && f; };
  int i = 42; float f = .13f;
  /* not constexpr */ auto value = csl::ag::to_tuple(A{ .i = i, .f = std::move(f) });

  [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
      ((std::cout << std::get<indexes>(value) << ' '), ...);
  }(std::make_index_sequence<csl::ag::size_v<A>>{});

  static_assert(std::same_as<
      int&,
      std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
  >);
  static_assert(std::same_as<
      float&&,
      std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
  >);
  ```
  
  </td><td>
  
  ```
  42 0.13 
  ```
  
  </td></tr></table>

#### Non-owning conversion (view, lightweight accessor)

This factory (as a function) creates non-owning **lightweight accessors** (views),  
returning a non-owning tuple (`std::tuple` of references), for which each element represents an accessor to an aggregate value's field.

Note that in order to preserve `cvref semantic`, the possibly-used `cvref qualifiers` of the aggregate's value are propagated to qualify each of non-ref-qualified elements of the result tuple-type.  
Ref-qualified fields type remain unchanged.

The conversion's result type can be access using the `tuple_view(_t)<T>` type-trait.

```cpp
struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };
int i = 42;

{ // using a rvalue
    [[maybe_unused]] auto view = csl::ag::to_tuple_view(type{ i, i, i, std::move(i) });

    static_assert(std::same_as<
        decltype(view),
        csl::ag::tuple_view_t<type&&>
    >);
    static_assert(std::same_as<
        decltype(view),
        std::tuple<int&&, int&, const int&, int&&>
        //         ^^^^^ cvref-qualified (rvalue-ref) propagation
    >);
}

{ // using a const-lvalue
    const auto & value = type{ i, i, i, std::move(i) };
    [[maybe_unused]] auto view = csl::ag::to_tuple_view(value);

    static_assert(std::same_as<
        decltype(view),
        csl::ag::tuple_view_t<const type&>
    >);
    static_assert(std::same_as<
        decltype(view),
        std::tuple<const int &, int&, const int &, int&&>
        //         ^^^^^^^^^^^ cvref-qualified (const-lvalue-ref) propagation
    >);
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/39bTrKzzo).

Additionally, `csl::ag::view_element(_t)<N,T>` can be used to obtains a field's type information, by index.

```cpp
struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };

// field 0 IS NOT a reference : cvref-qualifiers propagation
static_assert(std::same_as<int &&,
    csl::ag::view_element_t<0, type&&>
>);
static_assert(std::same_as<int &,
    csl::ag::view_element_t<0, type&>
>);
static_assert(std::same_as<const int &&,
    csl::ag::view_element_t<0, const type&&>
>);
static_assert(std::same_as<const int &,
    csl::ag::view_element_t<0, const type&>
>);

// field 0 IS a reference : no cvref-qualifiers propagation
static_assert(std::same_as<int &,
    csl::ag::view_element_t<1, type&&>
>);
static_assert(std::same_as<int &,
    csl::ag::view_element_t<1, type&>
>);
static_assert(std::same_as<int &,
    csl::ag::view_element_t<1, const type&&>
>);
static_assert(std::same_as<int &,
    csl::ag::view_element_t<1, const type&>
>);

// still not propagation for fields 2 and 3 ...
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/M3ejaf7Mc).

### tuplelike interface

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

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/YPj7931b9).

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

```text
42, 0.13
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/je4Gr16h5).

Slightly more advanced example :

```cpp
struct A{ int i; float f; };
auto value = A{ .i = 42, .f = 0.13f };

[&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    ((std::cout << std::get<indexes>(value) << ' '), ...);
}(std::make_index_sequence<csl::ag::size_v<A>>{});
```

```text
42 0.13 
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/j9bhr4WrP).

Note that `constexpr`-ness is preserved :

```cpp
struct A{ int i; char c; };
constexpr auto value = A{ 42, 'c' };
static_assert(csl::ag::get<0>(value) == 42);    // pass
static_assert(csl::ag::get<1>(value) == 'c');   // pass
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/h9jbrc8d6).

### Functional API

#### csl::ag::apply

Analogous to [`std::apply`](https://en.cppreference.com/w/cpp/utility/apply), but operates directly on an aggregate - unpacking its fields as arguments to a callable, without a prior `to_tuple` conversion.

```cpp
struct A{ int i; float f; };
auto value = A{ .i = 42, .f = 0.13f };

auto result = csl::ag::apply([](auto && ... fields){
    return (fields + ...);
}, value);
// result == 42.13f
```

#### csl::ag::for_each

Invokes a callable once per field, in order.

```cpp
struct A{ int i; float f; };
auto value = A{ .i = 42, .f = 0.13f };

csl::ag::for_each([](auto && field){
    std::cout << field << ' ';
}, value);
// output: 42 0.13
```

### Formatting and printing

There are two way to pretty-print aggregate types :

- using the legacy C++'s way : `std::ostream& operator<<(std::ostream&, T&&)` overload
- using the `fmt` or `std::format` library

#### using std::ostream

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

```text
A && : {
   [0] int : 42
   [1] float : 0.13
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/q8Yeq4e83).

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

```text
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

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/hsofqExoT).

#### using fmt

Requires `CSL_AG__ENABLE_FMTLIB_SUPPORT` (opt-in, off by default).  
Enable via CMake or by defining the macro before including the header:

```cpp
#define CSL_AG__ENABLE_FMTLIB_SUPPORT true
#include <csl/ag.hpp>
#include <fmt/format.h>
```

> If `fmt::fmt-header-only` is not available as a CMake target, it will be fetched automatically via `FetchContent`.  
> fmtlib >= 11 is required for the `:n` specifier.

This specializes `fmt::formatter<T>` for any `csl::ag::concepts::aggregate T` whose fields are all formattable.

Three format modes are available:

| Format string                              | Output style                                                 |
| ------------------------------------------ | ------------------------------------------------------------ |
| `"{}"`                                     | compact - fields wrapped in `{}`, nested aggregates recursed |
| `"{:n}"`                                   | no outer brackets                                            |
| `"{}"` on `value \| csl::ag::io::indented` | indented multi-line                                          |

Example:

```cpp
struct point { int x; int y; };
struct rect  { point top_left; point bottom_right; };

constexpr auto r = rect{ .top_left = { 0, 0 }, .bottom_right = { 10, 5 } };

fmt::print("{}\n",   r);                         // compact
fmt::print("{:n}\n", r);                         // no brackets
fmt::print("{}\n",   r | csl::ag::io::indented); // indented
```

Compact (`{}`):

```text
{{0, 0}, {10, 5}}
```

No brackets (`{:n}`):

```text
{0, 0}{10, 5}
```

Indented (`| csl::ag::io::indented`):

```text
{
    {
        0,
        0
    },
    {
        10,
        5
    }
}
```

Mixed-content aggregates (containing tuple-likes and ranges) are also supported - tuples render as `(...)`, arrays and other ranges as `[...]`.

## Homogeneity API with tuple-likes

As is, it is quite easy to handle aggregates and tuple in an homogeneous way, despite limitation listed in the next section below.

```cpp
void do_stuff_with_either_a_tuple_or_aggregate(csl::ag::concepts::structured_bindable auto && value) {

    using value_type = std::remove_cvref_t<decltype(value)>;

    using namespace csl::ag::tuplelike; // size, get, element

    constexpr auto size = size_v<value_type>;

    const auto do_stuffs = [&]<size_t index>(){

        auto && element_value = get<index>(std::forward<decltype(value)>(value));
        using element_value_type = decltype(element_value);
        using element_type = element_t<index, value_type>;

        // do stuffs with element_value, element_type ...
    };

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((do_stuffs.template operator()<indexes>()), ...);  
    }(std::make_index_sequence<size>{});
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/8fv1rfK6s).

## Current limitations

As-is, this implementation internally relies on structured-binding, which design choice expose two main limitations :

- Compile-time evaluation is limited.
- By-default behaviors injections, using STL extension/customization point (e.g injecting in the `std` namespace definitions for `get`/`tuple_element`/`tuple_size(_v)` won't work).
- Aggregate types with more fields than their size are currently not supported, but that's non-sense from a design perspective anyway.
- Ill-formed aggregate types using union-fields are not supported

## (Internal details) Where's the magic ?

Every magic has its own dirty secrets under the hood, and this library is no exception.  

Internally, the library determines the field count of a given aggregate type by probing `aggregate_constructible_from_n_values<T, N>` - whether `T` is constructible from `N` implicitly-castable-to-anything values - via `csl::ag::details::probing`.

Two strategies are used depending on the type:

### Fast path - for default_initializable<T> without bitfield support

`f(N) = aggregate_constructible_from_n_values<T, N>` is monotone (true for all N ≤ field_count, false beyond), so:

1. **Exponential probe**: start at N=1 and double until `f(2N)` is false or the cap is exceeded - O(log field_count) instantiations.
2. **Binary search** in the bracketed range - O(log field_count) instantiations.

Total cost: **O(log field_count)** template instantiations.

### Slow path - for non default_initializable<T> or when bitfield support is enabled

Monotonicity cannot be assumed, so binary search would be unsafe. Instead, a linear descent from a tight upper bound is used:

- Without bitfields: upper bound is `sizeof(T) / alignof(T)`.
- With bitfields: upper bound is `sizeof(T) / alignof(T) * CHAR_BIT`.

See `csl::ag::concepts::aggregate_constructible_from_n_values<T, N>` and `csl::ag::details::probing`.

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

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/v91bqTEWP).

---

## Example

@include overview.cpp

[ce-icon]: https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true
