# Overall presentation

The goal of `csl::ag` is to offer convenient ways to manipulate aggregate types.

This library is divided in five parts :

- Structured-binding for aggregate types
- A (non-owning) to-tuple conversion for aggregate types
- A tuplelike interface for aggregates types
- Aggregates-related type-traits
- (WIP) Pretty-printing (using `std::ostream & operator<<` overloads or `fmt`)

## Philosophy & design choices

The key idea of this library is to ease iterations over aggregates's member-variables,  
which is especially convenient when dealing with **reflection** and **serialization**.

- `csl::ag::size` (or `std::tuple_size_v` after a `to_tuple` conversion) give the fields count in a given aggregate type type
- `csl::ag::get<N>` (when N is a `std::size_t`) allow per-field access, in a similar way to `std::tuple` using `std::get<N>`

## Details

Unless otherwise specified, the following aggregate type - and associated value - will be used in the examples below:

```cpp
struct type_0{ int i = 0; char c = 'a'; };
auto value = type_0{ 42, 'A' }; // NOLINT
```

### Structured-binding for aggregate types

```cpp
[[maybe_unused]] auto && [ v0, v1 ] = value;
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

Try both examples on [compiler-explorer here](https://godbolt.org/z/z8vnxr619)).

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

#### `std::get`

Simple example (try it on [compiler-explorer here](https://godbolt.org/z/je4Gr16h5)) :

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

Advanced example (try it on [compiler-explorer here](https://godbolt.org/z/j9bhr4WrP)) :

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

### Pretty-printing

There are two way to pretty-print aggregate types :

- using the legacy C++'s way : `std::ostream& operator<<(std::ostream&, T&&)` overload
- using the `fmt` or `std::format` library

#### using `std::ostream` :

Simple example (Try it on [compiler-explorer here](https://godbolt.org/z/q8Yeq4e83)) :

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

Advanced example (try it on [compiler-explorer here](https://godbolt.org/z/hsofqExoT)) :

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
