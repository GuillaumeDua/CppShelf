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
- https://godbolt.org/z/rfee1Mn9E
- MVE issue: https://godbolt.org/z/eco1Pnszb

---

**Question**: `tuple_view` -> reference_collapsing of owner cvref + field cvref

- `std::get` propagates the owner's cvref-qualifier

    ```cpp
    template <class T, class... Types>
    constexpr T& get( tuple<Types...> & t) noexcept;
    template <class T, class... Types >
    constexpr T&& get( tuple<Types...> && t) noexcept;
    template <class T, class... Types >
    constexpr const T& get( const tuple<Types...> & t) noexcept;
    template <class T, class... Types >
    constexpr const T&& get( const tuple<Types...> && t) noexcept;
    ```

So a `tuple_view` must be consumed using the same cvref semantic as an owning `tuple`.  
(or should I implement a view_get, as partially-specializing `std::get` is not much of a good idea: `cert-dcl58-cpp` ...).

- ✅ Good `make_tuple_view(tuplelike&&)` implementation [here](https://godbolt.org/z/b135YPnMh).
  - using [quick DSL](https://godbolt.org/z/EsEva354T)

WIP: https://godbolt.org/z/59rMGMYMx
MVE: https://godbolt.org/z/6jjM5Tb4b
MVE + small tuple factory : https://godbolt.org/z/ahheTT8T9

### dev sample

```cpp
struct aggregate_type        { int v0; int & v1; int && v2; const int v3; const int & v4; const int && v5; };
using tuple_type = std::tuple< int,    int &,    int &&,    const int,    const int &,    const int && >;
```

### `std::forward_as_tuple`

Live demo [here](https://godbolt.org/z/4xbbY8fPE).

```console

value:       aggregate_type &
view :       std::tuple<int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: std::tuple<int, int &, int &&, const int, const int &, const int &&> &

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
       int & |        int & | ✅
       int & |        int & | ✅
       int & |        int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅

value:       const aggregate_type &
view :       std::tuple<const int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: const std::tuple<int, int &, int &&, const int, const int &, const int &&> &

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
 const int & |  const int & | ✅
       int & |        int & | ✅
       int & |        int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅

value:       aggregate_type &&
view :       std::tuple<int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: std::tuple<int, int &, int &&, const int, const int &, const int &&> &&

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
      int && |        int & | ❌
       int & |        int & | ✅
      int && |        int & | ❌
const int && |  const int & | ❌
 const int & |  const int & | ✅
const int && |  const int & | ❌

value:       const aggregate_type &&
view :       std::tuple<const int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: const std::tuple<int, int &, int &&, const int, const int &, const int &&> &&

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
const int && |  const int & | ❌
       int & |        int & | ✅
      int && |        int & | ❌
const int && |  const int & | ❌
 const int & |  const int & | ✅
const int && |  const int & | ❌

```

### `field_view`

Live demo [here](https://godbolt.org/z/Y89bEzM1f).

```cpp
// field_view
template <typename owner, typename T>
struct field_view : copy_cvref<owner, T>{};
template <typename owner, typename T>
requires (std::is_reference_v<T>)
struct field_view<owner, T> : std::type_identity<T>{};
template <typename owner, typename T>
using field_view_t = typename field_view<owner, T>::type;
```

```console

value:       aggregate_type &
view :       std::tuple<int &, int &, int &&, const int &, const int &, const int &&>
tuple_value: std::tuple<int, int &, int &&, const int, const int &, const int &&> &

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
       int & |        int & | ✅
       int & |        int & | ✅
       int & |        int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅

value:       const aggregate_type &
view :       std::tuple<const int &, int &, int &&, const int &, const int &, const int &&>
tuple_value: const std::tuple<int, int &, int &&, const int, const int &, const int &&> &

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
 const int & |  const int & | ✅
       int & |        int & | ✅
       int & |        int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅
 const int & |  const int & | ✅

value:       aggregate_type &&
view :       std::tuple<int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: std::tuple<int, int &, int &&, const int, const int &, const int &&> &&

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
      int && |        int & | ❌
       int & |        int & | ✅
      int && |        int & | ❌
const int && |  const int & | ❌
 const int & |  const int & | ✅
const int && |  const int & | ❌

value:       const aggregate_type &&
view :       std::tuple<const int &&, int &, int &&, const int &&, const int &, const int &&>
tuple_value: const std::tuple<int, int &, int &&, const int, const int &, const int &&> &&

       tuple |  tuple_view  |  same ?
       ----- |  ----------  |  ----
const int && |  const int & | ❌
       int & |        int & | ✅
      int && |        int & | ❌
const int && |  const int & | ❌
 const int & |  const int & | ✅
const int && |  const int & | ❌

```

### tuple view factory

// WIP. TODO: forward

```cpp
namespace mp {
template <template <typename ...> typename trait, typename ... bound_Ts>
struct bind_front {
// TODO: universal template argument
      template <typename ... Ts>
      using type = typename trait<bound_Ts..., Ts...>::type;

      template <typename ... Ts>
      constexpr static auto value = trait<bound_Ts..., Ts...>::value;
};
}

template <template <typename ...> typename trait, typename ... bound_Ts>
[[nodiscard]] auto make_tuple(mp::bind_front<trait, bound_Ts...>, auto && ... values){
    using type_factory = typename mp::bind_front<trait, bound_Ts...>;
    using result_type = std::tuple<
        typename type_factory::template type<decltype(values)>...
    >;
    return result_type{
        static_cast<typename type_factory::template type<decltype(values)>>(values)...
    };
}
```

## Questions

- explicit user-defined conversion from aggregate to tuple ? to tuple_view ?
- using DSL ?
  - `aggregate_value | as_tuple`
  - `aggregate_value | as_tuple_view`
