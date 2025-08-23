# WIP

- Remove all `tuple_` prefixes
- trim off the excess: remote what useless after refacto
- tuple_element_value: `decltype( get<I>(std::declval<T>()))`
  - which (if any) cvref-qualifier for T ? -> T&
- make_from_tuple
  - std::reference_constructs_from_temporary
- tuple: construct, assign from tuple-like
  - add some `from_tuple` tag to disambiguate constructions, with a specific user-defined deduction guide ?
    - What about `<tuple>` consistency then ?

Check:

- https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2481r2.html
  - T auto &&, &&&

## Documentation

- Better logo ?
- README
  - Design manifesto
    - coupled with `<utility>` as minimalistic impact
    - interops with STL's tuplelikes
      - `cat(tuple<>, std::tuple<>, std::array<>, std::pair<>)`
      - construct, assign, etc.
    - tuple
      - no recursion (tuple_cat, etc.)
      - decoupled from `<tuple>`
      - drop-in replacement for `<tuple>` with CSL_MP_TUPLE__IMPLICIT_CONVERSION == UNSAFE
        - CSL_MP_TUPLE__IMPLICIT_CONVERSION == SAFE -> `-Wconversion`
        - CSL_MP_TUPLE__IMPLICIT_CONVERSION == NONE -> strict, user-side conversions - if any
      - `std::` interops
      - `O(N)` access, lookup
      - IDEA: if quadtree-like composition, then minimalistic footprint and faster access ?
    - ttps mp algos on `tuple<ttps...>` vs. `mp::pack<ttps...>`
      - Better lookup perfs in some cases
      - same perfs for other cases ?
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

- range-like API for sequence, tuple
  - views-like composition: `std::make_index_sequence<4> | reverse | drop(1) | take(2)` => `std::integer_sequence<std::size_t, 3, 2>`
  - ranges-like mp algo `op<tuple-like, args...>`
    - `flatten_t<tuple<int>, tuple<tuple<char>>>` -> `tuple<int, char>`
    - `filter_if_t<tuple<float, int, double, char>, is_integral>` -> `tuple<int, char>`
  - vs.? consteval ranges-like algo `op(tuple-like &&, args&&...)`
    - `filter_if(tuple{A, B, C}, is_alive)` -> `tuple<A, C>`
    - `cat(tuple-like &&...)`
- Reverse API so it looks like ranges: tuple as first arg

- Which algorithms ? -> WIP: which ct (`name_t<tuple-like>`), which rt (`name(tuple-like &&, args...)`)
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

### Tests

- tuple
  - Check value semantic -> cvref qualifiers, unwanted copies, etc.
    - construct, assign, convert
  - Check conversions
