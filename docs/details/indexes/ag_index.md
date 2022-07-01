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
[[maybe_unused]] auto as_tuple = csl::ag::as_tuple(value);


```

### Pretty-printing

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
