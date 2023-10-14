# `csl::ag` cvref-qualifiers propagation

## Context: `std::tuple` cvref-qualifiers propagation

For

- `using tuple_type = /*possibly cvref-qualified */ std::tuple<Ts...>`

Reference-collapsing applies between:

- `std::tuple_element_t<index, tuple_type>`
- and `decltype(std::get<T>(std::declval<tuple_type>()))`

> See [demo here](https://godbolt.org/z/ahaWc1jT8).

## Context: aggregate

For a given aggregate type `A` like:

```cpp
struct A {
    T           v0;
    T &         v1;
    T &&        v2;
    const T     v3;
    const T &   v4;
    const T &&  v5;
};
```

we want it to be the equivalent of :

```cpp
std::tuple<int, int &, int &&, const int &, const int &, const int &&>
```

## `std::get` cvref-qualifiers propagation / semantic

- `std::get` triggers reference-collapsing: propagates the owner's cvref-qualifier through invocation, which might collapse with element's (possibly cvref-qualified) type.

```cpp
// by type
template <class T, class... Types>
constexpr T&
get(tuple<Types...> & t) noexcept;
template <class T, class... Types>
constexpr T&&
get(tuple<Types...> && t) noexcept;
template <class T, class... Types>
constexpr const T&
get(const tuple<Types...> & t) noexcept;
template <class T, class... Types>
constexpr const T&&
get(const tuple<Types...> && t) noexcept;

// by index
template <std::size_t I, class... Types>
constexpr typename std::tuple_element<I, tuple<Types...>>::type &
get(tuple<Types...> & t) noexcept;
template <std::size_t I, class... Types>
constexpr typename std::tuple_element<I, tuple<Types...>>::type &&
get(tuple<Types...> && t) noexcept;
template <std::size_t I, class... Types>
constexpr typename std::tuple_element<I, tuple<Types...>>::type const &
get(const tuple<Types...> & t) noexcept;
template <std::size_t I, class... Types>
constexpr typename std::tuple_element<I, tuple<Types...>>::type const &&
get(const tuple<Types...>&& t) noexcept;
```

- [demo](https://godbolt.org/z/edjcnczcq)

**Conclusion**: `tuple_view` -> reference_collapsing of owner cvref + field cvref

So a `tuple_view` must be consumed using the same cvref semantic as an owning `tuple`.  
*(or should I implement a view_get, as partially-specializing `std::get` is not much of a good idea: `cert-dcl58-cpp` ...)*.

## WIP

- https://godbolt.org/z/WsozGT8Eb
- MVE issue: https://godbolt.org/z/xdeqs3rjv
- https://godbolt.org/z/rfee1Mn9E
- MVE issue: https://godbolt.org/z/eco1Pnszb

---

- ✅ Good `make_tuple_view(tuplelike&&)` implementation [here](https://godbolt.org/z/b135YPnMh).
  - using [quick DSL](https://godbolt.org/z/EsEva354T)

WIP: https://godbolt.org/z/59rMGMYMx
MVE: https://godbolt.org/z/6jjM5Tb4b
MVE + small tuple factory : https://godbolt.org/z/ahheTT8T9

ISSUE: view and tuple for `std::get<T>`: types might occure more than once

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

### tuple view factory

**WIP**, implementation

- [here](https://godbolt.org/z/6Gb1PK5rv)
- https://godbolt.org/z/xTfaz9TP6
- IT WORKS ! 😍 https://godbolt.org/z/Wec3eGYGf

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
[[nodiscard]] auto tuple_utils::project/*transform*/(mp::bind_front<trait, bound_Ts...>, auto && ... values){
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

## Resources

Enriching type modification traits

- initial [P1016](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1016r0.pdf)
- superseed [P1450](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1450r3.pdf)
