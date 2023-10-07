# `csl::ag` cvref-qualifiers propagation

## Context: `std::tuple` cvref-qualifiers propagation

For

- `using tuple_type = /*possibly cvref-qualified */ std::tuple<Ts...>`

Reference-collapsing applies between:

- `std::tuple_element_t<index, tuple_type>`
- and `decltype(std::get<T>(std::declval<tuple_type>()))`

> See [demo here](https://godbolt.org/z/ahaWc1jT8).

## Context: aggregate

For a given type `A` like:

```cpp
struct A {
    T           v0;
    T&          v1;
    T&&         v2;
    const T     v3;
    const T &   v4;
    const T &&  v5;
}
```

## `csl::ag::element` / `std::tuple_element_t`

- transparent ?

|     ?      |  A  | A&  | A&& | const A | const A & | const A && |
| ---------- | --- | --- | --- | ------- | --------- | ---------- |
| T          |     |     |     |         |           |            |
| T&         |     |     |     |         |           |            |
| T&&        |     |     |     |         |           |            |
| const T    |     |     |     |         |           |            |
| const T &  |     |     |     |         |           |            |
| const T && |     |     |     |         |           |            |

## `decltype(std::get<T>(aggregate_value))` / `std::get<std::size_t>(tupleview_value)`

|     ?      |  T  | T&  | T&& | const T | const T & | const T && |
| ---------- | --- | --- | --- | ------- | --------- | ---------- |
| T          |     |     |     |         |           |            |
| T&         |     |     |     |         |           |            |
| T&&        |     |     |     |         |           |            |
| const T    |     |     |     |         |           |            |
| const T &  |     |     |     |         |           |            |
| const T && |     |     |     |         |           |            |

## Resources

Enriching type modification traits

- initial [P1016](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1016r0.pdf)
- superseed [P1450](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1450r3.pdf)

## WIP

- https://godbolt.org/z/WsozGT8Eb
- MVE issue: https://godbolt.org/z/xdeqs3rjv

## Questions

- explicit user-defined conversion from aggregate to tuple ? to tuple_view ?
