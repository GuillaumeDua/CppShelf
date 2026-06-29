
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

<!-- EXAMPLE_BEGIN: 01_overview_demo.cpp -->
```cpp
#include <csl/typeinfo.hpp> // optional: gives csl::ag::io::typenamed clean type names (e.g. "int")
#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT 1
#include <csl/ag.hpp>
#include <iostream> // std::print might not be available yet: use `std::cout << std::format(...)`

struct S { char c; int i; };

static_assert(
    csl::ag::concepts::aggregate<S> and
    csl::ag::size_v<S> == 2
);
static_assert(std::same_as<char, csl::ag::element_t<0, S>>);
static_assert(std::same_as<int,  csl::ag::element_t<1, S>>);

auto main() -> int {
    auto value = S{ .c='A', .i=41 }; // NOLINT
    ++csl::ag::get<1>(value);

    using namespace csl::ag::io;
    constexpr auto format_options = indexed | typenamed | indented;
    std::cout << std::format("{}", value | format_options); // equivalent to std::println("{:xit}", value)

    // legacy alternative: CSL_AG__ENABLE_IOSTREAM_SUPPORT
    // std::cout << "value: " << format_options << value << '\n';

    // other alternative: CSL_AG__ENABLE_FMT_SUPPORT
    // fmt::println("{:xit}", value)
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/zW31zz3fh)
<!-- EXAMPLE_END: 01_overview_demo.cpp -->

Output:

```text
value: {
    [0] char: 'A',
    [1] int: 42
}
```

## Introduction

By default, the C++ standard allow structured-binding for aggregate types.

<!-- EXAMPLE_BEGIN: 02_structured_binding.cpp -->
```cpp
#include <csl/ag.hpp>
#include <cassert>

auto main() -> int {
    struct type { int i; char c; };
    auto value = type{ 42, 'A' }; // NOLINT

    [[maybe_unused]] auto && [v0, v1] = value;
    assert(v0 == 42);
    assert(v1 == 'A');
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/3jrKPhT8P)
<!-- EXAMPLE_END: 02_structured_binding.cpp -->

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

The key idea of this library is to ease iterations over aggregates's member-variables in C++20 (so, without reflection),  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size<T>` gives the fields count in a given aggregate type type  
  (or [std::tuple_size_v](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size) after a `to_tuple` or `to_tuple_view` conversion)
- `csl::ag::get<size_t N>(aggregate auto && value)` allows per-field access, in a similar way to [std::get<N>](https://en.cppreference.com/w/cpp/utility/tuple/get) for [std::tuple<Ts...>](https://en.cppreference.com/w/cpp/utility/tuple)

---

## Getting starting

This library is single-header, header-only.  
Users may use it in various ways, however [CMake](https://cmake.org/) is the promoted one for both download and configuration.

> ℹ️ The `typenamed` formatting option (see [formatting and printing](#formatting-and-printing)) optionally enhances itself
> with [csl::typeinfo](https://github.com/GuillaumeDua/CppShelf/blob/main/libs/typeinfo/includes/typeinfo/csl/typeinfo.hpp)
> for compile-time type names, detected via `__has_include`.  
> Without it, a `<typeindex>`-based runtime (inconsistent) fallback is used instead - `ag.hpp` stays usable as a single, standalone header either way.

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

Using features for this library with any aggregate type using **custom layout** will results in ☣️ **undefined behavior**.  
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

#### Formatting and printing

> 💡 Everything related to formatting and printing lives in the `csl::ag::io` namespace.

> 💡 All options in this section are opt-ins *(`OFF` by default)*, and can be combined.

Supports 3 parallel backends:

- `std::format`
- `fmt`
- `std::ostream` (e.g `std::cout`)

Format-specs are composable, and include `indexed` (":x"), `indented` (":i"), and `typenamed` (":t").

```cpp
struct A{ int i{}; };
struct my_aggregate{ char c = 'a'; A a = A{ .i = 13} };
constexpr auto my_aggregate_value = my_aggregate{};
```

- `CSL_AG__ENABLE_STD_FORMAT_SUPPORT`: add `std::formatter<csl::ag::aggregate T>`

  ```cpp
  std::print("{}",     my_aggregate_value);                         // compact, default → {'a', {13}}
  std::print("{:n}",   my_aggregate_value);                         // no outer brackets → 'a'{13}
  std::print("{:xti}", my_aggregate_value);                         // format-spec letters: x=indexed, t=typenamed, i=indented
  std::print("{}",     my_aggregate_value | csl::ag::io::indented); // pretty, multi-line → see below

  using namespace csl::ag::io;
  constexpr auto view = indexed | typenamed | indented;             // composed format_options, reusable
  std::print("{}", my_aggregate_value | view);                      // view-based composition → same as "{:xti}"
  ```

- `CSL_AG__ENABLE_FMTLIB_SUPPORT`: makes `csl::ag` depends on the `fmt` library, and add `fmt::formatter<csl::ag::aggregate T>`.

  > If [fmtlib](https://github.com/fmtlib/fmt)'s `cmake` target `fmt::fmt-header-only` is not available when building `csl::ag` with `CSL_AG__ENABLE_FMTLIB_SUPPORT` set to `ON`, then such a dependency will be injected using `cmake FetchContent`.

  ```cpp
  fmt::print("{}",     my_aggregate_value);
  fmt::print("{:n}",   my_aggregate_value);
  fmt::print("{:xti}", my_aggregate_value); // format-spec letters: x=indexed, t=typenamed, i=indented
  fmt::print("{}",     my_aggregate_value | csl::ag::io::indented);

  using namespace csl::ag::io;
  constexpr auto view = indexed | typenamed | indented; // composed format_options, reusable
  fmt::print("{}", my_aggregate_value | view);          // view-based composition → same as "{:xti}"
  ```

- `CSL_AG__ENABLE_IOSTREAM_SUPPORT`: add `csl::ag::io::operator<<(std::ostream &, csl::ag::concepts::structured_bindable auto const &)`

  ```cpp
  using namespace csl::ag::io;
  std::cout << my_aggregate_value;              // compact, default
  std::cout << no_braces << my_aggregate_value; // one-shot manipulator → no outer brackets
  std::cout << (my_aggregate_value | indented); // view-based composition → pretty, multi-line
  ```

- `csl::ag::io::to_string`: available as soon as any one of the options above is enabled. Returns a `std::string` directly, with the same call syntax regardless of which backend is active (favors `std::format`, then `fmt::format`, then `std::ostream`).

  ```cpp
  using namespace csl::ag::io;
  to_string(my_aggregate_value);                                    // compact, default     → {'a', {13}}
  to_string<no_braces>(my_aggregate_value);                         // no outer brackets    → 'a'{13}
  to_string<indented>(my_aggregate_value);                          // pretty, multi-line   → see below
  to_string<indexed | typenamed | indented>(my_aggregate_value);    // pretty, multi-line, indexed, typenamed
  ```

About compact vs. pretty presentations, with type names (`typenamed`):

- Compact (`typenamed`):

```text
{char: 'a', A: {int: 13}}
```

- Pretty (`typenamed | indented`):

```text
{
    char: 'a',
    A: {
        int: 13
    }
}
```

## Content

All components that are part of the public interface are defined in the namespace `csl::ag`.

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

<!-- EXAMPLE_BEGIN: 03_size.cpp -->
```cpp
#include <csl/ag.hpp>

struct A { int i; float f; };
static_assert(csl::ag::size<A>::value == 2);
static_assert(csl::ag::size_v<A>      == 2);

auto main() -> int {}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/61PonYhxh)
<!-- EXAMPLE_END: 03_size.cpp -->

Just like `std::tuple_size`/`std::tuple_size_v`, the **value** can be accessed using a convenience alias :

```cpp
template <csl::ag::concepts::aggregate T>
constexpr inline static auto size_v = size<T>::value;
```

#### csl::ag::element<std::size_t, concepts::aggregate>

Type-identity of a field's type of a given aggregate type.

<!-- EXAMPLE_BEGIN: 04_element.cpp -->
```cpp
#include <csl/ag.hpp>

struct A { int i; float f; };
static_assert(std::same_as<int,   csl::ag::element_t<0, A>>);
static_assert(std::same_as<float, csl::ag::element_t<1, A>>);

auto main() -> int {}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/8jcGdPfav)
<!-- EXAMPLE_END: 04_element.cpp -->

Just like `std::tuple_element/std::tuple_element_t`, the **type** can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using element_t = typename element<N, T>::type;
```

#### csl::ag::view_element<std::size_t, concepts::aggregate>

In a similar way to `csl::ag::element<std::size_t, T>`, `csl::ag::view_element<std::size_t,T>` is a type-identity for a field's type of a given aggregate view type.  
For more details about aggregate's view, see the [to-tuple non-owning conversion (view)](#non-owning-conversion-view-lightweight-accessor) section.

<!-- EXAMPLE_BEGIN: 05_view_element.cpp -->
```cpp
#include <csl/ag.hpp>

struct A { int i; float & f; const char && c; };

static_assert(std::same_as<int&&,        csl::ag::view_element_t<0, A&&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, A&&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, A&&>>);

static_assert(std::same_as<const int&,   csl::ag::view_element_t<0, const A&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, const A&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, const A&>>);

auto main() -> int {}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/xazP1vKds)
<!-- EXAMPLE_END: 05_view_element.cpp -->

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

<!-- EXAMPLE_BEGIN: 06_to_tuple_owning.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>
#include <utility>

struct A { int i; float f; };

auto main() -> int {
    constexpr auto value = A{ .i = 42, .f = 0.13f };
    constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        static_assert((std::same_as<
            csl::ag::element_t<indexes, A>,
            std::tuple_element_t<indexes, std::remove_cvref_t<decltype(value_as_tuple)>>
        > and ...));
        ((std::cout << std::get<indexes>(value_as_tuple) << ' '), ...);
    }(std::make_index_sequence<csl::ag::size_v<A>>{});
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/cqhP7xf1d)
<!-- EXAMPLE_END: 06_to_tuple_owning.cpp -->

Output:

```text
42 0.13 
```

The main advantage here is to use such function in a `constexpr` contexts.  
A precondition while doing so is that each aggregates field's value must be usable in a constexpr context though (e.g **not** ref-qualified).

<!-- EXAMPLE_BEGIN: 07_to_tuple_type_traits.cpp -->
```cpp
#include <csl/ag.hpp>
#include <utility>

struct A { int i; float f; };

static_assert(std::same_as<
    std::tuple<int, float>,
    csl::ag::to_tuple_t<A>
>);

static_assert(std::same_as<int, csl::ag::element_t<0, A>>);
static_assert(std::same_as<int, std::tuple_element_t<0, csl::ag::to_tuple_t<A>>>);
static_assert(std::same_as<int, std::tuple_element_t<0, decltype(csl::ag::to_tuple(A{}))>>);

auto main() -> int {
    constexpr auto value = A{ .i = 42, .f = 0.13f };
    constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

    static_assert(42    == std::get<0>(value_as_tuple));
    static_assert(0.13f == std::get<1>(value_as_tuple));
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/WWYfj97Gv)
<!-- EXAMPLE_END: 07_to_tuple_type_traits.cpp -->

Additionaly, [std::tuple_element_t](https://en.cppreference.com/w/cpp/utility/tuple/tuple_element) can be use to obtains the conversion result's element types.

- Example 1 : aggregate type with not-cvref-qualified fields

<!-- EXAMPLE_BEGIN: 08_to_tuple_example1.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
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
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/6se87rMsT)
<!-- EXAMPLE_END: 08_to_tuple_example1.cpp -->

Output:

```text
42 0.13 
```

- Example 2 : aggregate type with ref-qualified fields

<!-- EXAMPLE_BEGIN: 09_to_tuple_example2.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>
#include <utility>

auto main() -> int {
    struct A { int & i; float && f; };
    int i = 42; float f = .13f;
    auto value = csl::ag::to_tuple(A{ .i = i, .f = std::move(f) });

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
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/zWr4fTe1K)
<!-- EXAMPLE_END: 09_to_tuple_example2.cpp -->

Output:

```text
42 0.13 
```

#### Non-owning conversion (view, lightweight accessor)

This factory (as a function) creates non-owning **lightweight accessors** (views),  
returning a non-owning tuple (`std::tuple` of references), for which each element represents an accessor to an aggregate value's field.

Note that in order to preserve `cvref semantic`, the possibly-used `cvref qualifiers` of the aggregate's value are propagated to qualify each of non-ref-qualified elements of the result tuple-type.  
Ref-qualified fields type remain unchanged.

The conversion's result type can be access using the `tuple_view(_t)<T>` type-trait.

<!-- EXAMPLE_BEGIN: 10_to_tuple_view.cpp -->
```cpp
#include <csl/ag.hpp>
#include <utility>

auto main() -> int {

    struct type {
        int value; int & lvalue; const int & const_lvalue; int && rvalue; // NOLINT(*-avoid-const-or-ref-data-members)
        [[nodiscard]] static auto make(int & i){ return type{ .value=i, .lvalue=i, .const_lvalue=i, .rvalue=std::move(i) }; }
    };
    int i = 42; // NOLINT(*-avoid-magic-numbers)

    { // rvalue source: non-ref fields acquire rvalue-ref qualification
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(type::make(i));

        static_assert(std::same_as<
            decltype(view),
            csl::ag::view_t<type&&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<int&&, int&, const int&, int&&>
        >);
    }

    { // const-lvalue source: non-ref fields acquire const-lvalue-ref qualification
        const auto & value = type::make(i); // lifetime extension
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(value);

        static_assert(std::same_as<
            decltype(view),
            csl::ag::view_t<const type&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<const int&, int&, const int&, int&&>
        >);
    }
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/eMM3P9GTz)
<!-- EXAMPLE_END: 10_to_tuple_view.cpp -->

Additionally, `csl::ag::view_element(_t)<N,T>` can be used to obtains a field's type information, by index.

<!-- EXAMPLE_BEGIN: 11_view_element_cvref.cpp -->
```cpp
#include <csl/ag.hpp>

struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };

// field 0 is NOT a reference: cvref-qualifier of the aggregate propagates
static_assert(std::same_as<int &&,       csl::ag::view_element_t<0, type&&>>);
static_assert(std::same_as<int &,        csl::ag::view_element_t<0, type&>>);
static_assert(std::same_as<const int &&, csl::ag::view_element_t<0, const type&&>>);
static_assert(std::same_as<const int &,  csl::ag::view_element_t<0, const type&>>);

// field 1 IS a reference: no cvref-qualifier propagation
static_assert(std::same_as<int &, csl::ag::view_element_t<1, type&&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, type&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, const type&&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, const type&>>);

auto main() -> int {}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/vGYhPPGxv)
<!-- EXAMPLE_END: 11_view_element_cvref.cpp -->

### tuplelike interface

#### std::tuple_element

<!-- EXAMPLE_BEGIN: 12_tuple_element.cpp -->
```cpp
#include <csl/ag.hpp>

auto main() -> int {

    struct type { const int i = 0; char & c; }; // NOLINT
    char c = 'c';
    auto value = type{ 42, c }; // NOLINT

    static_assert(std::same_as<
        const int,
        csl::ag::element_t<0, std::remove_cvref_t<decltype(value)>>
    >);
    static_assert(std::same_as<
        char&,
        csl::ag::element_t<1, std::remove_cvref_t<decltype(value)>>
    >);
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/o1z7s7r6z)
<!-- EXAMPLE_END: 12_tuple_element.cpp -->

#### std::get

Simple example:

<!-- EXAMPLE_BEGIN: 13_get_simple.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {

    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    // output: "42, 0.13"
    std::cout << csl::ag::get<0>(value) << ", " << csl::ag::get<1>(value) << '\n';

    static_assert(std::same_as<int &,   decltype(csl::ag::get<0>(value))>);
    static_assert(std::same_as<float &, decltype(csl::ag::get<1>(value))>);
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/1e3vWhhx5)
<!-- EXAMPLE_END: 13_get_simple.cpp -->

Output:

```text
42, 0.13
```

Slightly more advanced example:

<!-- EXAMPLE_BEGIN: 14_get_advanced.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    // output: "42 0.13 "
    [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((std::cout << csl::ag::get<indexes>(value) << ' '), ...);
    }(std::make_index_sequence<csl::ag::size_v<A>>{});
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/3GP7eGWa6)
<!-- EXAMPLE_END: 14_get_advanced.cpp -->

Output:

```text
42 0.13 
```

Note that `constexpr`-ness is preserved :

<!-- EXAMPLE_BEGIN: 15_get_constexpr.cpp -->
```cpp
#include <csl/ag.hpp>

struct A { int i; char c; };
constexpr auto value = A{ 42, 'c' };
static_assert(csl::ag::get<0>(value) == 42);
static_assert(csl::ag::get<1>(value) == 'c');

auto main() -> int {}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/hrnzGjj3W)
<!-- EXAMPLE_END: 15_get_constexpr.cpp -->

### Functional API

#### csl::ag::apply

Analogous to [`std::apply`](https://en.cppreference.com/w/cpp/utility/apply), but operates directly on an aggregate - unpacking its fields as arguments to a callable, without a prior `to_tuple` conversion.

<!-- EXAMPLE_BEGIN: 16_apply.cpp -->
```cpp
#include <csl/ag.hpp>

auto main() -> int {

    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    auto result = csl::ag::apply([](auto && ... fields){
        return (static_cast<float>(fields) + ...);
    }, value);
    static_assert(std::same_as<float, decltype(result)>);
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/GbEjrKhPf)
<!-- EXAMPLE_END: 16_apply.cpp -->

#### csl::ag::for_each

Invokes a callable once per field, in order.

<!-- EXAMPLE_BEGIN: 17_for_each.cpp -->
```cpp
#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13f };

    csl::ag::for_each([](auto && field){
        std::cout << field << ' ';
    }, value);
    std::cout << '\n';
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/baff8dTzc)
<!-- EXAMPLE_END: 17_for_each.cpp -->

Output:

```text
42 0.13 
```

### Formatting and printing

There are three ways to pretty-print aggregate types :

- (✅ Best) using `std::format` or the `fmt` library
- using the C++'s legacy way : `std::ostream& operator<<(std::ostream&, T&&)` overload
- using `csl::ag::io::to_string<format_options>`, a homogeneous (yet, less efficient) API across all of the above

#### using std::format

Requires `CSL_AG__ENABLE_STD_FORMAT_SUPPORT` (opt-in, off by default).  
Enable via CMake or by defining the macro before including the header:

```cpp
#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT true
#include <csl/ag.hpp>
#include <format>
```

This specializes `std::formatter<T>` for any `csl::ag::concepts::aggregate T` whose fields are all formattable.

Three format modes are available:

| Format string                              | Output style                                                 |
| ------------------------------------------ | ------------------------------------------------------------ |
| `"{}"`                                     | compact - fields wrapped in `{}`, nested aggregates recursed |
| `"{:n}"`                                   | no outer brackets                                            |
| `"{}"` on `value \| csl::ag::io::indented` | indented multi-line                                          |

Example:

```cpp
struct point     { int x; int y; };
struct rectangle { point top_left; point bottom_right; };

constexpr auto r = rectangle{
    .top_left = { 0, 0 },
    .bottom_right = { 10, 5 }
};

using namespace csl::ag::io;
// NOTE: prefer std::print, if available
std::cout
    << std::format("{}\n",   r)                                     // compact
    << std::format("{:n}\n", r)                                     // no brackets
    << std::format("{}\n",   r | indented)                          // indented
    << std::format("{}\n",   r | indexed | typenamed | indented)    // indexed, typenamed, indented
;
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

Indexed and typenamed and indented (`| indexed | typenamed | indented`):

```text
{
    [0] point: {
        [0] int: 0,
        [1] int: 0
    },
    [1] point: {
        [0] int: 10,
        [1] int: 5
    }
}
```

Mixed-content aggregates (containing tuple-likes, ranges, etc.) are also supported - tuples render as `(...)`, arrays and other ranges as `[...]`, etc.

#### using fmt

Requires `CSL_AG__ENABLE_FMTLIB_SUPPORT` (opt-in, off by default) - same macro-driven setup as `std::format` above:

```cpp
#define CSL_AG__ENABLE_FMTLIB_SUPPORT true
#include <csl/ag.hpp>
#include <fmt/format.h>
```

> If `fmt::fmt-header-only` is not available as a CMake target, it will be fetched automatically via CMake's `FetchContent`.  
> fmtlib >= 11 is required for the `:n` specifier.

`fmt::formatter<T>` / `fmt::format` / `fmt::print` behave identically to their `std::formatter` / `std::format` / `std::print` counterparts described above: same format modes, same composable `csl::ag::io::indented | indexed | typenamed` options, same output.  

#### using std::ostream

Simple example :

<!-- EXAMPLE_BEGIN: 18_formatting_ostream_simple.cpp -->
```cpp
#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; float f; };
    std::cout << A{ .i = 42, .f = .13F };
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/Wve3j19b9)
<!-- EXAMPLE_END: 18_formatting_ostream_simple.cpp -->

Output:

```text
{42, 0.13}
```

Advanced example :

<!-- EXAMPLE_BEGIN: 19_formatting_ostream_advanced.cpp -->
```cpp
#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#include <csl/ag.hpp>
#include <array>
#include <iostream>
#include <string>
#include <tuple>

struct A { int i; float f; };
struct B {};

// Note: must be in the same namespace as B (global here) so ADL finds it
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

auto main() -> int {
    using namespace csl::ag::io;

    int i = 42;
    char c = 'c';
    auto value = C{
        .a = A{ .i=13, .f=.12F },
        .b = B{},
        .i = i, .str = "str", .c = std::move(c)
    };
    std::cout << value;
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/MY7PYePTd)
<!-- EXAMPLE_END: 19_formatting_ostream_advanced.cpp -->

Output:

```text
{{13, 0.12}, user-defined operator<<(std::ostream&, const B &), 42, "str", 'c', (true, 2), ['a', 'b', 'c']}
```

#### using `to_string<format_options>`

`csl::ag::io::to_string` returns a `std::string` directly, with an consistent/homogeneous API regardless of which formatting support is enabled,  
favoring `std::format`, then `fmt::format`, then `std::ostream`.

Options are selected as a non-type template argument. Tags (`indented`, `no_braces`, `indexed`, `typenamed`) implicitly convert to `format_options` and compose via `operator|`, so they can be combined directly as the template argument - or, equivalently, via the view-based `operator|` on the value itself:

```cpp
#include <csl/ag.hpp>

struct point { int x; int y; };
struct rect  { point top_left; point bottom_right; };

auto main() -> int {
    using namespace csl::ag::io;

    constexpr auto r = rect{ .top_left = { 0, 0 }, .bottom_right = { 10, 5 } };

    to_string(r);                     // "{{0, 0}, {10, 5}}"        (default, compact)
    to_string<no_braces>(r);          // "{0, 0}{10, 5}"            (no outer brackets)
    to_string<indented>(r);           // multiline, see below
    to_string<indented | indexed>(r); // composed options, as NTTP
    to_string(r | indented | indexed); // equivalent, view-based composition
}
```

`to_string<indented>(r)`:

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

## Homogeneity API with tuple-likes

As is, it is quite easy to handle aggregates and tuple in an homogeneous way, despite limitation listed in the next section below.

<!-- EXAMPLE_BEGIN: 20_homogeneity.cpp -->
```cpp
#include <csl/ag.hpp>
#include <tuple>

void do_stuff(csl::ag::concepts::structured_bindable auto && value) {
    using value_type = std::remove_cvref_t<decltype(value)>;
    using namespace csl::ag::tuplelike;

    constexpr auto sz = size_v<value_type>;

    const auto process = [&]<std::size_t index>(){
        [[maybe_unused]] auto && field = get<index>(std::forward<decltype(value)>(value));
        using field_type = element_t<index, value_type>;
        (void)static_cast<field_type *>(nullptr);
    };

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((process.template operator()<indexes>()), ...);
    }(std::make_index_sequence<sz>{});
}

auto main() -> int {
    struct A { int i; float f; };
    do_stuff(A{ .i = 1, .f = 2.f });
    do_stuff(std::tuple<int, float>{ 1, 2.f });
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/hdM61xWbj)
<!-- EXAMPLE_END: 20_homogeneity.cpp -->

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

<!-- EXAMPLE_BEGIN: 21_internal_size.cpp -->
```cpp
#include <csl/ag.hpp>

auto main() -> int {
    struct A { char a, b, c, d, e, f, g, h; };
    static_assert(sizeof(A) == 8);
    static_assert(csl::ag::size_v<A> == 8);

    struct B { int a, b; };
    static_assert(sizeof(B) == 8);
    static_assert(csl::ag::size_v<B> == 2);

    struct alignas(32) C { char c; };
    static_assert(sizeof(C) == 32);
    static_assert(csl::ag::size_v<C> == 1);
}
```

[![CE][ce-icon] Try me on compiler-explorer](https://godbolt.org/z/9MhdYn9dh)
<!-- EXAMPLE_END: 21_internal_size.cpp -->

---

## Example

@include overview.cpp

[ce-icon]: https://github.com/GuillaumeDua/CppShelf/blob/main/doc/details/images/compiler-explorer.png?raw=true
