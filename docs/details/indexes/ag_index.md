# Overall presentation

The goal of `csl::ag` is to offer convenient ways to manipulate aggregate types.

This library is divided in five parts :

- Structured-binding for aggregate types
- A (non-owning) to-tuple conversion for aggregate types
- A tuplelike interface for aggregates types
- Aggregates-related type-traits
- (WIP) Pretty-printing (using `std::ostream operator<<` or `fmt`)

## Philosophy & design choices

The key idea of this library is to ease iterations over aggregates's member-variables,  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size` (or `std::tuple_size_v` after a `to_tuple` conversion) give the fields count in a given aggregate type type
- `csl::ag::get<N>` (when N is a `std::size_t`) allow per-field access, in a similar way to `std::tuple` using `std::get<N>`

## Details

Considering the following aggregate type, and associated value :

```cpp
struct type_0{ int i = 0; char c = 'a'; };
auto value = type_0{ 42, 'A' }; // NOLINT
```

### Structured-binding for aggregate types

```cpp
[[maybe_unused]] auto && [ v0, v1 ] = value;
```

### (non-owning) to-tuple conversion for aggregate types

```cpp
[[maybe_unused]] auto as_tuple = csl::ag::as_tuple(value); // not constexpr yet
static_assert(std::same_as<
  decltype(as_tuple),
  std::tuple<int&, char&>
>);
```

### tuplelike interface for aggregates

#### `std::tuple_element`

```cpp
struct type{ const int i = 0; char & c; };
char c = 'c';
auto value = type_4{ 42, c }; // NOLINT

static_assert(std::same_as<
    const int,
    std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
>);
static_assert(std::same_as<
    char&,
    std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
>);
```

#### `std::get`

// TODO


### Pretty-printing

There are two way to pretty-print aggregate types :

- using the legacy C++'s way : `std::ostream& operator<<(std::ostream&, T&&)` overload
- using the `fmt` or `std::format` library

#### using `std::ostream` :

Simple example :

Try this on [compiler-explorer here](https://godbolt.org/z/q8Yeq4e83).

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

Advanced example :

Try this on [compiler-explorer here](https://godbolt.org/z/hsofqExoT).

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

## `std::tuple` and aggregate types homogeneity

As is, it is quite easy to handle aggregates and tuple in an homogeneous way, despite limitation listed in the next section below.

```cpp
void do_stuff_with_either_a_tuple_or_aggregate(csl::ag::concepts::structured_bindable auto && value) {

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
    using element_type = std::tuple_element_t<std::remove_cvref_t<decltype(value)>>;

    // do stuffs with element_value, element_type ...
  };

  [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
      ((do_stuffs.template operator()<indexes>()), ...);  
  }(std::make_index_sequence<size>{});
}
```

###

## Current limitations

As-is, this implementation internally relies on structured-binding, which design choice expose two main limitations :

- Compile-time evaluation is limited.
- By-default behaviors injections, using STL extension/customization point (e.g injecting in the `std` namespace definitions for `get`/`tuple_element`/`tuple_size(_v)` won't work).
