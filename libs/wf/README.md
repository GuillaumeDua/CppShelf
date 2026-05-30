# csl::wf

Generic eDSL builder - a workflow / pipeline composition library for C++20.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

> **Note**: this library is no longer under active development. A refactor or cleanup may happen at some point, but no new features are under active plannification.

## Include

```cpp
#include <csl/wf.hpp>
```

---

## Design overview

`csl::wf` extends standard invocation (`std::invoke`, `std::apply`) to handle functors whose `operator()` is **templated** or has **multiple overloads**, where the standard library's type-erasing approach (`std::function`) falls short.

Two tag types carry the extra information through the API:

| Type              | Purpose                                                                  |
| ----------------- | ------------------------------------------------------------------------ |
| `mp::ttps<Ts...>` | Pack of **template type parameters** to pass to a templated `operator()` |
| `mp::args<Ts...>` | Pack of **argument types** (used with `is_invocable_with`)               |

---

## Invocation

### `invoke`

```cpp
csl::wf::invoke<ttps...>(f, args...);
csl::wf::invoke(f, mp::ttps<Ts...>{}, args...);
```

Extends `std::invoke` to support explicit template parameters on `operator()`:

```cpp
auto f = []<typename T>(T v) { return v * 2; };

csl::wf::invoke<int>(f, 21);      // calls f.operator()<int>(21) -> 42
csl::wf::invoke(f, 21);           // calls std::invoke(f, 21)
```

### `apply` / `apply_before` / `apply_after`

Tuple-unpacking variants of `invoke`:

```cpp
auto args = std::tuple{21};

csl::wf::apply(f, args);                        // f(21)
csl::wf::apply_before(f, args, 100);            // f(21, 100)  - tuple args first
csl::wf::apply_after(f, args, 100);             // f(100, 21)  - tuple args last
```

All accept an optional leading `ttps<...>` argument.

---

## Invocation traits - `csl::wf::mp`

`ttps`-aware counterparts to the standard `<type_traits>` invocability traits:

| Trait                                                      | Description                                                |
| ---------------------------------------------------------- | ---------------------------------------------------------- |
| `is_invocable<F, [ttps<...>,] args...>` / `_v`             | Whether `F` is invocable with optional template parameters |
| `is_nothrow_invocable<...>` / `_v`                         | Same, `noexcept`                                           |
| `is_invocable_r<R, F, ...>` / `_v`                         | With return type check                                     |
| `is_nothrow_invocable_r<R, F, ...>` / `_v`                 | Both                                                       |
| `invoke_result<F, ...>` / `_t`                             | Deduced return type                                        |
| `is_applyable<F, [ttps<...>,] tuple>` / `_v`               | Whether `apply(f, tuple)` is valid                         |
| `is_applyable_before<F, ttps<...>, tuple, args...>` / `_v` | `apply_before` validity                                    |
| `is_applyable_after<F, ttps<...>, tuple, args...>` / `_v`  | `apply_after` validity                                     |

Combined pack form (takes both `ttps<...>` and `args<...>` as single types):

| Trait                                                       | Description                           |
| ----------------------------------------------------------- | ------------------------------------- |
| `is_invocable_with<F, ttps<...>, args<...>>` / `_v`         | Invocability check with packed params |
| `is_nothrow_invocable_with<F, ttps<...>, args<...>>` / `_v` | `noexcept` variant                    |

---

## Binders

Type-correct alternatives to `std::bind_front` / `std::bind_back` that also support binding template type parameters.

### `bind_front` / `bind_back`

```cpp
auto add = [](int a, int b) { return a + b; };

auto add5 = csl::wf::bind_front(add, 5);
add5(3);   // -> 8

auto add5_back = csl::wf::bind_back(add, 5);
add5_back(3);  // -> 8  (3 + 5)
```

With template parameters:

```cpp
auto typed_add = []<typename T>(T a, T b) { return a + b; };
auto int_add = csl::wf::bind_front<int>(typed_add);
int_add(1, 2);  // calls typed_add.operator()<int>(1, 2)
```

The underlying `front_binder<F, ttps<...>, args<...>>` and `back_binder` types are also available for direct use.

---

## Non-owning wrappers

### `function_view<F>`

Stores `F` by **lvalue reference**. Cheaper than `std::reference_wrapper` - passes `operator()` through all cvref-qualifications and supports `ttps`.

```cpp
auto f = [](int x) { return x * 2; };
csl::wf::function_view view{f};
view(21);  // -> 42
```

### `function_ref<F>`

Stores `F` by **pointer**. Rebindable, swappable, nullable-checked on construction.

```cpp
csl::wf::function_ref ref{f};
ref(21);  // -> 42
```

Both throw `std::invalid_argument` if constructed from `nullptr`.

---

## Repetition

### `invoke_n_times<N>(f, args...)`

Calls `f(args...)` exactly `N` times. Returns `void` if `f` returns `void`, otherwise an `std::array<result_t, N>`.

```cpp
csl::wf::invoke_n_times<3>([]{ std::puts("hello"); });
// prints "hello" three times

auto results = csl::wf::invoke_n_times<3>([](int x){ return x * 2; }, 21);
// results == std::array{42, 42, 42}
```

### `repeater<N, F>`

A callable wrapper that repeats its call `N` times on each `operator()` invocation. Repeated `repeater`s flatten their counts multiplicatively.

```cpp
using namespace csl::wf::literals;

auto r = csl::wf::make_repetition<3>([]{ std::puts("tick"); });
r();  // prints "tick" three times

// or with the _times literal:
auto r2 = []{ std::puts("tick"); } * 3_times;
r2();
```

---

## Route - pipeline composition

`route<Fs...>` chains a sequence of callables: each step receives the return value of the previous step as its argument. Steps that don't accept the current value may discard it (with `allow_discard`) or must accept it (`nodiscard`).

```cpp
auto pipeline = csl::wf::make_continuation(
    [](int x)    { return x * 2; },
    [](int x)    { return std::to_string(x); },
    [](std::string s) { return s + "!"; }
);

pipeline(21);  // -> "42!"
```

`route` is also constructible directly:

```cpp
csl::wf::route r{
    [](int x) { return x + 1; },
    [](int x) { return x * 3; }
};
r(4);  // -> 15
```

Access individual nodes with `r.at<N>()` or `csl::wf::get<N>(r)`.

---

## eDSL operators

Import with `using namespace csl::wf::operators;`:

| Expression    | Result                                                   |
| ------------- | -------------------------------------------------------- |
| `f >>= g`     | `make_continuation(f, g)` - creates or extends a `route` |
| `f \| g`      | `make_condition(f, g)` - creates an overload set         |
| `f * N_times` | `make_repetition<N>(f)` - creates a `repeater`           |
| `f \| ref`    | `function_ref{f}`                                        |
| `f \| cref`   | `function_ref<const F>{f}`                               |
| `f \| view`   | `function_view{f}`                                       |

```cpp
using namespace csl::wf::operators;
using namespace csl::wf::literals;

auto process =
    [](int x)   { return x * 2; }
    >>= [](int x) { return std::to_string(x); }
    >>= [](std::string s) { return s + "!"; };

process(21);  // -> "42!"

auto handle = [](int x) { return x + 1; }
            | [](std::string s) { return s.size(); };

handle(41);          // -> 42
handle("hello");     // -> 5

auto repeat = [](int x) { return x * 2; } * 3_times;
repeat(5);  // -> std::array{10, 10, 10}
```

---

## Chain traits - `csl::wf::mp::chain_trait<Fs...>`

Compile-time introspection of a pipeline:

```cpp
using pipe = csl::wf::mp::chain_trait<
    decltype([](int) { return 0.0f; }),
    decltype([](float) { return std::string{}; })
>;

static_assert(pipe::is_invocable<int>);
static_assert(std::same_as<pipe::invoke_result_t<int>, std::string>);
```

---

## Example

@include overview.cpp
