# csl::mp

C++ template metaprogramming utilities for C++20/23.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

## Include

```cpp
#include <csl/mp.hpp>
```

---

## tuple<Ts...>

A drop-in replacement for `std::tuple` with a richer access syntax and full STL interoperability (`std::get`, `std::tuple_size`, `std::tuple_element`, structured bindings).

```cpp
auto value = csl::mp::tuple{ 42, 3.14f, std::string{"hello"} };

value.get<0>();                     // by index
value.get<std::string>();           // by type
value[csl::mp::index_t<0>{}];       // operator[] by index
value[std::type_identity<int>{}];   // operator[] by type
```

Factory functions mirror the standard:

| Function                    | Description                            |
| --------------------------- | -------------------------------------- |
| `make_tuple(args...)`       | Decay-copies arguments                 |
| `tie(lvalues...)`           | Creates a tuple of lvalue references   |
| `forward_as_tuple(args...)` | Preserves value categories             |
| `cat(tuples...)`            | Concatenates any number of tuple-likes |

---

## Algorithms - csl::mp::functions

All algorithms operate on a value of any `tuple_like` type (including `csl::mp::tuple`, `std::tuple`, `std::pair`, `std::array`).

| Function                        | Description                                              |
| ------------------------------- | -------------------------------------------------------- |
| `for_each(t, f)`                | Visits each element with `f(element)`                    |
| `for_each_enumerate(t, f)`      | Visits with `f(index, element)` - index as runtime value |
| `for_each_enumerate_nttp(t, f)` | Visits with `f<index>(element)` - index as NTTP          |
| `fold_left(t, f, init)`         | Left fold: `f(f(f(init, x0), x1), ..., xn)`              |
| `fold_right(t, f, init)`        | Right fold: `f(x0, f(x1, ..., f(xn, init)...))`          |
| `all_of(t, p)`                  | True if `p` holds for every element                      |
| `any_of(t, p)`                  | True if `p` holds for at least one element               |
| `none_of(t, p)`                 | True if `p` holds for no element                         |

---

## Type traits - csl::mp::type_traits

Trait composition utilities:

| Trait                      | Description                                              |
| -------------------------- | -------------------------------------------------------- |
| `negate<P>`                | Logical complement of a unary predicate                  |
| `bind_front<trait, Ts...>` | Partially applies `Ts...` as leading template arguments  |
| `bind_back<trait, Ts...>`  | Partially applies `Ts...` as trailing template arguments |
| `is_homogeneous<T>`        | Whether all elements of a tuple-like share the same type |
| `is_constrained_by<T, P>`  | Whether all elements satisfy predicate `P`               |

Reference-qualified cv manipulation (P1450): `copy_ref`, `add_const`, `add_volatile`, `add_cv` - preserve reference category across cv qualification.

### Tuples algorithms - csl::mp::type_traits

Compile-time operations on the element types of any `tuple_like`. All produce a `::type` alias and a corresponding `_t` shorthand.

**Query:**

| Trait                        | Description                                 |
| ---------------------------- | ------------------------------------------- |
| `contains<T, U>` / `_v`      | Whether type `U` appears in `T`             |
| `index_of<T, U>` / `_v`      | Index of the first occurrence of `U` in `T` |
| `last_index_of<T, U>` / `_v` | Index of the last occurrence of `U` in `T`  |
| `count<T, U>` / `_v`         | Number of `U` occurrences in `T`            |
| `count_if<T, P>` / `_v`      | Number of elements satisfying predicate `P` |
| `cat_result<Ts...>` / `_t`   | Result type of `cat(Ts{}...)`               |

**Transformation:**

| Trait                         | Description                                 |
| ----------------------------- | ------------------------------------------- |
| `filter<T, P>` / `_t`         | Keep elements satisfying predicate `P`      |
| `replace<T, From, To>` / `_t` | Replace all occurrences of `From` with `To` |
| `replace_if<T, P, To>` / `_t` | Replace elements satisfying `P` with `To`   |
| `unique<T>` / `_t`            | Remove duplicate element types              |
| `push_back<T, U>` / `_t`      | Append type `U`                             |
| `push_front<T, U>` / `_t`     | Prepend type `U`                            |
| `pop_back<T>` / `_t`          | Remove last element type                    |
| `pop_front<T>` / `_t`         | Remove first element type                   |

**Set operations:**

| Trait                             | Description                           |
| --------------------------------- | ------------------------------------- |
| `set_union<T1, T2>` / `_t`        | Types in `T1` or `T2` (no duplicates) |
| `set_intersection<T1, T2>` / `_t` | Types present in both `T1` and `T2`   |
| `set_difference<T1, T2>` / `_t`   | Types in `T1` that are not in `T2`    |

---

## Concepts - csl::mp::concepts

| Concept                                  | Description                               |
| ---------------------------------------- | ----------------------------------------- |
| `tuple_like<T>`                          | Satisfies the P2165 tuple-like interface  |
| `pair_like<T>`                           | `tuple_like` with exactly 2 elements      |
| `instance<ttp, T>`                       | `T` is an instantiation of template `ttp` |
| `non_narrowing_constructible_from<T, U>` | Construction without narrowing            |

---

## Sequences - csl::mp::concepts::sequence

Type traits (`csl::mp`): `is_sequence<T>` / `_v`, `is_index_sequence<T>` / `_v`, `reverse_t<T>`, `make_reverse_index_sequence<N>`, `to_tuplelike<T>` / `_t`.

Concepts:

| Concept                | Description                                     |
| ---------------------- | ----------------------------------------------- |
| `sequence<T>`          | `T` is a `std::integer_sequence` specialisation |
| `index_sequence<T>`    | `T` is a `std::index_sequence` specialisation   |
| `empty<T>`             | Sequence with zero elements                     |
| `not_empty<T>`         | Sequence with at least one element              |
| `sized<T, N>`          | Sequence with exactly `N` elements              |
| `sized_at_least<T, N>` | Sequence with at least `N` elements             |

---

## Standard library proposals implementations

> These are best-effort implementations sufficient for `csl::mp`'s own use. They may not fully conform to the final proposal wording or the standardised versions where those exist.  
> Each proposal lives in a dedicated sub-namespace named after its ID (e.g. `csl::mp::P0887`, `csl::mp::concepts::P2165`). Those under `csl::mp` are declared as `inline` namespaces and are therefore directly accessible as `csl::mp::type_identity`, `csl::mp::forward_like`, etc. Those under `csl::mp::concepts` are not inline and must be qualified accordingly.

| Proposal | Content                                                           |
| -------- | ----------------------------------------------------------------- |
| P0887    | `type_identity<T>`                                                |
| P0318    | `unwrap_reference`, `unwrap_ref_decay`                            |
| P1450    | Reference-aware `copy_ref`, `add_const`, `add_volatile`, `add_cv` |
| P2445    | `forward_like<T>(u)`                                              |
| P2165    | `tuple_like`, `pair_like`, `tuple_element` concepts               |

---

## Example

@include overview.cpp
