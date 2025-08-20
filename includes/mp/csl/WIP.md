# WIP

- Remove all `tuple_` prefixes
- trim off the excess: remote what useless after refacto
- tuple_element_value: `decltype( get<I>(std::declval<T>()))`
  - which (if any) cvref-qualifier for T ? -> T&
- make_from_tuple
  - std::reference_constructs_from_temporary
- CSL_MP_TUPLE__IMPLICIT_CONVERSION and/vs. `cast<tuplelike>(tuplike auto &&)`

Check:

- https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2481r2.html
  - T auto &&, &&&

## Documentation

- Better logo ?
- README
  - Design manifesto
    - tuple
      - no recursion (tuple_cat, etc.)
      - decoupled from `<tuple>`
      - drop-in replacement for `<tuple>`
        - And CSL_MP_TUPLE__IMPLICIT_CONVERSION values
      - `std::` interops
      - `O(N)` access, lookup
      - IDEA: if quadtree-like composition, then minimalistic footprint and faster access ?
- Doxygen

## Support

- cxx standard: 20
- MSVC
- gcc, clang min versions

## Test

- Coverage
- noexcept
  - construction
  - comparison

## Refacto

- naming: avoid redundancies
  - naming: `csl::mp::tpl` (after merge into `mp-tuple-storage`)
  - remove all `tuple_` prefix/suffix, but in `std` namespace
  - naming clash for algorithms
    - function vs. type_traits
  - remove all `_result` -> dedicated namespace

- piecewise: constructible, convertible
  - use in conversion, etc.

- have constructors, assign, destructors behave well by default,  
  so I don't need to implement each and every case
  - But what about conversions then ?
    - `std::tuple` drop-in replacement: Still don't wanna force conversions to be on the caller site
    - Yet, is the consumer responsible to `-w` or `-isystem` such a header ?

## Optimisation

- homogeneous layout -> array as storage
- a bit like `csl::ag` lookup
  - Hard-code 0..8 storage, get
  - some quadtree-like composition ?
    - `tuple<Ts[1..8], Ts[9..10]>`
- std::tuple_element already using inheritance
- Consider only using aggregate initialization
  - no constructors for tuple
  - no constructors for details::storage

## Features

- interops with tuplelikes
  - compare
  - conversion from/to std::tuple ? tuplelikes ?
    - make_from, convert_to ?
- for_each, `| indexed` ?
- apply
  - operator()
  - with detect/opt-in csl::functional arguments ?
- make_from
- print
  - `std::formatter`
  - `fmt::formatter`
  - `std::ostream<<`

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
