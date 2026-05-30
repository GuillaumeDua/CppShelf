# csl::functional

Function and overload traits for C++20/23.

Part of [CppShelf](https://github.com/GuillaumeDua/CppShelf) - a collection of single-header, header-only C++ libraries.

## Include

```cpp
#include <csl/functional.hpp>
```

---

## `arguments<Ts...>`

Represents a packed argument list as a single type, with a tuplelike interface.  
Enables argument lists to be stored, passed, and reasoned about as types rather than template parameter packs.

```cpp
using args_t = csl::functional::arguments<int, float, std::string>;
// equivalent to tuple-like<int, float, std::string>
static_assert(std::is_same_v<int, std::tuple_element_t<0, args_t>>);

auto args = args_t{ 42, 3.14f, "hello" };
auto result = std::apply([](int i, float f, std::string_view s) {
    return std::to_string(i + f) + std::string{s};
}, args);
```

---

## `overload<Ts...>`

Classic overload set pattern - inherits `operator()` from each `Ts`, creating a single callable that dispatches based on argument types.

```cpp
auto visitor = csl::functional::overload{
    [](int   v) { return v * 2; },
    [](float v) { return v + 0.5f; },
    [](auto  v) { return v; }   // fallback
};

visitor(42);     // -> int branch
visitor(1.5f);   // -> float branch
```

The deduction guide ensures clean construction from lambdas or any callable.

---

## `function_trait<F>`

Extracts the return type and argument list of any callable `F` as associated types.

| Member           | Type                                      |
| ---------------- | ----------------------------------------- |
| `result_type`    | Return type of `F`                        |
| `arguments_type` | `arguments<Args...>` - the parameter list |

> **Limitation**: `function_trait` cannot resolve overloaded free functions.  
> Taking the address of an overloaded function is ambiguous at the call site,  
> so there is no unique type to specialise on. Wrap the desired overload in a lambda to make it unambiguous:
>
> ```cpp
> // function_trait<decltype(std::to_string)>                                   // error: ambiguous
> function_trait<decltype([](int v){ return std::to_string(v); })>              // ok: lambda wrapper
> function_trait<decltype(static_cast<std::string(*)(int)>(std::to_string))>    // ok: static_cast
> ```

Supported callable forms:

| Form                    | Example                            |
| ----------------------- | ---------------------------------- |
| Free function           | `void(int, float)`                 |
| Function pointer        | `void(*)(int)`                     |
| Member function pointer | `void(Foo::*)(int) const noexcept` |
| Functor / lambda        | Any type with `operator()`         |
| `std::function`         | `std::function<int(double)>`       |

All `cv` and `noexcept` qualifications are handled.

```cpp
auto f = [](int x, float y) { return x + y; };

using trait = csl::functional::function_trait<decltype(f)>;
// trait::result_type    -> float
// trait::arguments_type -> arguments<int, float>
// std::tuple_element_t<0, trait::arguments_type> -> int
```

Convenience aliases:

```cpp
csl::functional::function_trait_result_t<decltype(f)>;    // float
csl::functional::function_trait_arguments_t<decltype(f)>; // arguments<int, float>
```

For member functions, the class type is prepended to `arguments_type` as the first element:

```cpp
struct Foo { int bar(double) const; };

using trait = csl::functional::function_trait<decltype(&Foo::bar)>;
// trait::result_type    -> int
// trait::arguments_type -> arguments<const Foo, double>
```

---

## Overload introspection

Three traits inspect an `overload<Ts...>` as a whole, returning `std::tuple`s indexed by overload member:

| Trait                          | Result                              |
| ------------------------------ | ----------------------------------- |
| `overload_trait<O>` / `_t`     | `tuple-like<function_trait<Ts>...>` |
| `overload_arguments<O>` / `_t` | `tuple-like<arguments_type<Ts>...>` |
| `overload_result<O>` / `_t`    | `tuple-like<result_type<Ts>...>`    |

```cpp
using O = csl::functional::overload<
    decltype([](int)   {}),
    decltype([](float) { return 1.0f; })
>;

using args    = csl::functional::overload_arguments_t<O>;
// tuple-like<arguments<int>, arguments<float>>

using results = csl::functional::overload_result_t<O>;
// tuple-like<void, float>
```

---

## Type traits - `csl::functional::type_traits`

Argument-pack-aware variants of the standard `<type_traits>` invocability traits. Instead of a variadic `Ts...`, they take an `arguments<Ts...>` type, enabling the argument list to be stored and forwarded as a single type.

| Trait                                                   | Description                                          |
| ------------------------------------------------------- | ---------------------------------------------------- |
| `is_invocable<F, arguments<Ts...>>` / `_v`              | Whether `F` is invocable with `Ts...`                |
| `is_nothrow_invocable<F, arguments<Ts...>>` / `_v`      | Whether that invocation is `noexcept`                |
| `is_invocable_r<R, F, arguments<Ts...>>` / `_v`         | Whether `F(Ts...)` returns a type convertible to `R` |
| `is_nothrow_invocable_r<R, F, arguments<Ts...>>` / `_v` | Combined `noexcept` + return check                   |
| `is_simple_callable<F>` / `_v`                          | Whether `F` has exactly one unambiguous `operator()` |

```cpp
using args = csl::functional::arguments<int, double>;
using F    = decltype([](int, double){});

static_assert(csl::functional::type_traits::is_invocable_v<F, args>);
static_assert(csl::functional::type_traits::is_simple_callable_v<F>);
```

`is_simple_callable` is false for `overload` (multiple `operator()`) and for function pointers that are overloaded at the call site.

---

## Concepts - `csl::functional::concepts`

| Concept                                     | Description                                    |
| ------------------------------------------- | ---------------------------------------------- |
| `invocable_with<F, arguments_type>`         | `F` is invocable with the packed argument list |
| `nothrow_invocable_with<F, arguments_type>` | Same, and the invocation is `noexcept`         |
| `simple_callable<F>`                        | `F` has a single unambiguous call operator     |

```cpp
template <csl::functional::concepts::simple_callable F>
void register_callback(F && f);
```
