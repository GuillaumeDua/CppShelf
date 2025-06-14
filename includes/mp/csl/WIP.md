# WIP

- storage: explicit, noexcept, etc.
- conditionally enable implicit conversions
  - `tuple_storage<indexes, Us...>`

- tuple_element_value: `decltype( get<I>(std::declval<T>()))`
  - which (if any) cvref-qualifier for T ? -> T&

## Refacto

- naming: `csl::mp::tpl` (after merge into `mp-tuple-storage`)
  - remove all `tuple_` prefix, but in `std` namespace
  - size
  - element
  - etc.

- piecewise: constructible, convertible
  - use in conversion, etc.

## Optimisation

- homogeneous layout -> array as storage
- a bit like `csl::ag` lookup
  - Hard-code 0..8 storage, get
  - some quadtree-like composition ?
    - `tuple<Ts[1..8], Ts[9..10]>`

## Features

- interops with tuplelikes
  - compare
  - conversion from/to std::tuple ? tuplelikes ?
    - make_from, convert_to ?

### Algorithms

- Reverse API so it looks like ranges: tuple as first arg
- front, back
  - function -> get
  - type_trait -> type
- apply
- Cartesian product ? (motivation: matrixes)
