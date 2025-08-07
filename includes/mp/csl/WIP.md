# WIP

- Remove all `tuple_` prefixes
- trim off the excess: remote what useless after refacto
- tuple_element_value: `decltype( get<I>(std::declval<T>()))`
  - which (if any) cvref-qualifier for T ? -> T&
- make_from_tuple
  - std::reference_constructs_from_temporary

Check:

- https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2481r2.html
  - T auto &&, &&&

## Test

- Doxygen
- Coverage
- noexcept
  - construction
  - comparison

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
- std::tuple_element already using inheritance

## Features

- interops with tuplelikes
  - compare
  - conversion from/to std::tuple ? tuplelikes ?
    - make_from, convert_to ?
- foreach
- apply
- make_from

### Algorithms

- Reverse API so it looks like ranges: tuple as first arg
- front, back
  - function -> get
  - type_trait -> type
- all/any/none
- split, chunk_by, extract
- filter, filter_if
- partition
- fold_left, fold_right
- zip(tuplelikes...)
- flatten -> see POC
- Cartesian product ? (motivation: matrixes)
- Sorting ?
