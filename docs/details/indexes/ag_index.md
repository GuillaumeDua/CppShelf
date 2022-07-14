# Overall presentation

The goal of `csl::ag` is to offer convenient ways to manipulate aggregate types.

## Introduction

By default, the C++ standard allow structured-binding for aggregate types.

```cpp
struct type{ int i; char c; };
auto value = type{ 42, 'A' }; // NOLINT

[[maybe_unused]] auto && [ v0, v1 ] = value;
assert(v0 == 42);   // pass
assert(v1 == 'A');  // pass
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/3EcK9Wc7h).

However, there is no - *simple* - way to access the following informations for a given aggregate type or value :

- The quantity of fields
- Access a field's value anonymously
- Iterate over fields

This library provides a way to obtain such information, and internally use it to provide convenient high-level conversions and printing functions.

---

This library is divided in four distinct parts :

- [#1](#aggregate-related-type-traits) Aggregates-related type-traits
- [#2](#to-tuple-conversion-for-aggregate-types) to-tuple conversion for aggregate types *(owning or not)*
- [#3](#tuplelike-interface-for-aggregates) A tuplelike interface for aggregates types
- [#4 (WIP)](#pretty-printing) Pretty-printing (using `std::ostream & operator<<` overloads or `fmt`)

---

## Philosophy & design choices

The key idea of this library is to ease iterations over aggregates's member-variables,  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size` (or `std::tuple_size_v` after a `to_tuple` conversion) give the fields count in a given aggregate type type
- `csl::ag::get<N>` (when N is a `std::size_t`) allow per-field access, in a similar way to `std::tuple` using `std::get<N>`

## Details


### Aggregate-related type-traits

#### `csl::ag::size<T>`

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

#### `csl::ag::element<std::size_t, T>`

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

#### `csl::ag::view_element`

In a similar way to `csl::ag::element<std::size_t, T>`, `csl::ag::view_element<std::size_t,T>` is a type-identity for a field's type of a given aggregate view type.  
For more details about aggregate's view, see the [to-tuple non-owning conversion (view)](#non-owning-conversion-view) section.

```cpp
struct A{ int i; float f; };
static_assert(std::same_as<int&&,   csl::ag::view_element_t<0, A>>);
static_assert(std::same_as<float&&, csl::ag::view_element_t<1, A>>);
```

[<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> Try me on compiler-explorer](https://godbolt.org/z/xMYzezxoo).

The **type** can be accessed using a convenience alias :

```cpp
template <std::size_t N, concepts::aggregate T>
using view_element_t = typename view_element<N, T>::type;
```

### to-tuple conversion for aggregate types

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

### tuplelike interface for aggregates

#### `std::tuple_element`

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

#### `std::get`

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

#### using `std::ostream` :

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
