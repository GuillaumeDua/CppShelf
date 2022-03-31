# Documentation : csl::wf

The goal of `csl::wf` is to offer convenient ways to manipulate functors & callable values in general.

This library is divided in three parts :

- Invocation [**functions**](#invocation-utilities) *(`invoke`, `apply`, etc.)*,  
  and [**type-traits**](#invocation-traits) *(`is_invocable`, `is_applyable`, `is_nothrow_invocable_r`, etc.)*
- Functors [**types**]() with specific purpose *(front_binder, overload, repeater, etc.)*
- An [**eDSL**](#edsl) to create complexe workflow easily

---

- [Documentation : csl::wf](#documentation--cslwf)
  - [Philosophy & design choices](#philosophy--design-choices)
  - [Invocation utilities](#invocation-utilities)
    - [**invoke**](#invoke)
    - [**apply**](#apply)
    - [apply_before](#apply_before)
    - [apply_after](#apply_after)
  - [Invocation traits](#invocation-traits)
    - [is_invocable](#is_invocable)
    - [is_nothrow_invocable](#is_nothrow_invocable)
    - [is_invocable_r](#is_invocable_r)
    - [is_nothrow_invocable_r](#is_nothrow_invocable_r)
    - [invoke_result](#invoke_result)
    - [is_applyable](#is_applyable)
    - [is_nothrow_applyable](#is_nothrow_applyable)
    - [is_applyable_before](#is_applyable_before)
    - [is_nothrow_applyable_before](#is_nothrow_applyable_before)
    - [is_applyable_after](#is_applyable_after)
    - [is_nothrow_applyable_after](#is_nothrow_applyable_after)
    - [apply_result](#apply_result)
    - [is_invocable_with](#is_invocable_with)
    - [is_nothrow_invocable_with](#is_nothrow_invocable_with)
  - [bind_front](#bind_front)
    - [front_bindable (concept)](#front_bindable-concept)
    - [front_binder (type)](#front_binder-type)
  - [invocation policies](#invocation-policies)
    - [csl::wf::invoke_policy::allow_discard](#cslwfinvoke_policyallow_discard)
    - [csl::wf::invoke_policy::nodiscard](#cslwfinvoke_policynodiscard)
  - [chain](#chain)
    - [`mp::chain_trait<typename ... Fs>`](#mpchain_traittypename--fs)
    - [chain_invoke](#chain_invoke)
    - [chain_invoke_nodiscard](#chain_invoke_nodiscard)
    - [chain_invocable (concept)](#chain_invocable-concept)
    - [route (type)](#route-type)
      - [get<index>(instance_of<route>)](#getindexinstance_ofroute)
  - [overload](#overload)
  - [repeater](#repeater)
  - [function_ref<T>](#function_reft)
  - [function_view<T>](#function_viewt)
  - [eDSL](#edsl)
    - [_times (literals)](#_times-literals)
    - [factories](#factories)
      - [make_continuation](#make_continuation)
      - [make_condition](#make_condition)
      - [make_repetition](#make_repetition)
    - [operators](#operators)
      - [`operator|` (pipe operator)](#operator-pipe-operator)
        - [F1 | F2](#f1--f2)
        - [F | ref](#f--ref)
        - [F | cref](#f--cref)
        - [F | view](#f--view)
      - [`operator>>=` (right-shift-assign operator)](#operator-right-shift-assign-operator)
      - [`operator*` (star operator)](#operator-star-operator)

## Philosophy & design choices

The key idea of this library is to provide a convenient way to deal with functors which `operator()` are conditionaly-generated, have multiple overloads, or simply have template-parameters.

> As-is, only template-type parameters are supported.

Such design is in opposition to STL's `<functional>`, where types like `std::function` provides a type-erasure restrained by a unique function signature.

To allow multiples templates-parameters-pack in signatures, and represent for instance in one hand template-type-parameters and generic-parameters on the other,  
a type named `ttps<...>` is used to ease deduction.  
For some specific corner-cases, a type named `arg<...>` is also used to wrap `parameters`'s types in the same way.

It also provides a precise support to `operator()` cvref-qualifiers.

## Invocation utilities

> Similar to the STL equivalents,  
> but offer two ways to pass template-type-parameters to the callable value.

- As template-type-parameters to the function itself

    ```cpp
    csl::wf::invoke<std::string>(func, 42);
    //             ^^^^^^^^^^^^^
    ```

- As an optional `csl::wf::ttps<...>` regular parameter

    ```cpp
    csl::wf::invoke(func, ttps<std::string>{}, 42);
    //                    ^^^^^^^^^^^^^^^^^^^
    ```

The correct invocation synthax is then deduced.

Note that all implementations are conditionaly enabled/disabled using proper concepts.  
Thus, invalid synthax will result in an invalid overload-resolution.

### **invoke**

Signatures :

```cpp
// (1) - similar to std::invoke
template <typename F, typename ... args_types>
constexpr decltype(auto) invoke(F && f, args_types&& ... args) noexcept(/**/);

// (2) - with template-type-parameter-pack
template <typename ... ttps_args, typename F, typename ... args_types>
constexpr decltype(auto) invoke(F && f, args_types&& ... args) noexcept(/**/);

// (3) - with template-type-parameter-pack as a function parameter of type csl::wf::mp::ttps<...>
template <typename ... ttps_args, typename F, typename ... args_types>
constexpr decltype(auto) invoke(F && f, mp::ttps<ttps_args...>, args_types&& ... args) noexcept(/**/);
```

Examples :

```cpp
constexpr auto func = []<typename T>(auto && value){};
func.template operator()<std::string>(42);  // regular function invocation

// std::invoke
std::invoke(&std::remove_cvref_t<decltype(func)>::template operator()<std::string, int>, func, 42);
//                                    notice the 2nd template-type-parameter here  ^^^

// csl::wf::invoke
csl::wf::invoke<std::string>(func, 42);         // (2) : template-type-parameters as invoke template-type-parameters
csl::wf::invoke(func, ttps<std::string>{}, 42); // (3) : or as 2nd optional parameters
```

Try it on [Godbolt here](https://godbolt.org/z/MbeqxEnG4)

### **apply**

> Similar to `std::apply`,  
> but provides two ways to pass template-type-parameters, in a similar fashion to `csl::wf::invoke`.

Signatures :

```cpp
// (1) - with template-type-parameter-pack
template <typename ... f_ts, typename F, concepts::tupleinterface_not_starting_with_ttps args_as_tuple_t>
constexpr decltype(auto) apply(F && f, args_as_tuple_t&& args) noexcept(/**/);

// (2) - with template-type-parameter-pack as first TupleType element
template <typename F, concepts::tupleinterface_starting_with_ttps args_as_tuple_t>
constexpr decltype(auto) apply(F && f, args_as_tuple_t && args) noexcept(/**/);
```

Examples :

```cpp
constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// std::apply
using namespace std::placeholders;
std::apply(
    std::bind(
        &std::remove_cvref_t<decltype(func)>::template operator()<std::string, char, int>,
        func,
        _1, _2
    ), 
    std::tuple{'A', 42}
);

// csl::wf::apply
csl::wf::apply<std::string>(func, std::tuple{'A', 42});             // (1)
csl::wf::apply(func, std::tuple{ ttps<std::string>{}, 'A', 42 });   // (2)

```

### apply_before

Apply the values contained in the tuple **before** optional additional values.

Signature :

```cpp
template <typename ... f_ts, typename F, concepts::tuple_interface args_as_tuple_t, typename ... func_args_t>
constexpr decltype(auto) apply_before(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args) noexcept(/**/)
```

Examples :

```cpp
constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// csl::wf::apply_before
csl::wf::apply_before<std::string>(func, std::tuple{ A{}, B{} });
csl::wf::apply_before<std::string>(func, std::tuple{ A{} }, B{});
csl::wf::apply_before<std::string>(func, std::tuple{}, A{}, B{});

csl::wf::apply_before(func, std::tuple{ ttps<std::string>{}, A{}, B{} });
csl::wf::apply_before(func, std::tuple{ ttps<std::string>{}, A{}},  B{});
csl::wf::apply_before(func, std::tuple{ ttps<std::string>{} }, A{}, B{} );
```

### apply_after

Apply the values contained in the tuple **after** optional additional values.

Signature :

```cpp
template <typename ... f_ts, typename F, concepts::tuple_interface args_as_tuple_t, typename ... func_args_t>
constexpr decltype(auto) apply_after(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args) noexcept(/**/)
```

```cpp
constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// csl::wf::apply_after
csl::wf::apply_after<std::string>(func, std::tuple{ A{}, B{} });
csl::wf::apply_after<std::string>(func, std::tuple{ B{} }, A{});
csl::wf::apply_after<std::string>(func, std::tuple{}, A{}, B{});

csl::wf::apply_after(func, std::tuple{ ttps<std::string>{}, A{}, B{} });
csl::wf::apply_after(func, std::tuple{ A{}, B{} }, ttps<std::string>{});
```

## Invocation traits

Available either as type-traits, or constexpr template-variable with `_v` prefix.

### is_invocable

```cpp
is_invocable<F, [ttps<...>,] args_types...>
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | template-type-parameters |
| `args_types` | Parameters types.<br>Must be complete types usable in an unevaluated context. |

---

Similar to [std::is_invocable](https://en.cppreference.com/w/cpp/types/is_invocable), but `F` can be a functor with multiples `operator()` overload, and supports template-type-parameters.
An additional non-mandatory (possibly cv-ref-qualified) `ttps<...>` parameter can be used to pass template-type-parameters. 

Determines whether 
- `F` can be invoked with non-mandatory template-type-parameter `ttps<...>`
- and arguments `args_types...`

> Note : `ttps<...>` can be **cvref-qualified**. If so, the behavior remain the same.

```cpp
auto func = [](){};

using F = decltype(func);
using namespace csl::wf;

static_assert(mp::is_invocable_v<F>);                   // evaluate func()
static_assert(mp::is_invocable_v<F, ttps<>>);           // evaluate func()
static_assert(mp::is_invocable_v<F, const ttps<> &&>);  // evaluate func()
```

```cpp
auto func = []<typename ... Ts>(auto && ... args){};

using F = decltype(func);
using namespace csl::wf;

static_assert(mp::is_invocable_v<F, int>);              // evaluate func(int{})
static_assert(mp::is_invocable_v<F, int, char>);        // evaluate func(int{}, char{})

static_assert(mp::is_invocable_v<F, ttps<>>);           // evaluate func<>()
static_assert(mp::is_invocable_v<F, ttps<>, int>);      // evaluate func<>(int{})
static_assert(mp::is_invocable_v<F, ttps<char>, int>);  // evaluate func<char>(int{})
```

### is_nothrow_invocable

```cpp
is_nothrow_invocable<F, [ttps<...>,] args_types...>
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | template-type-parameters |
| `args_types` | Parameters types.<br>Must be complete types usable in an unevaluated context. |

---

Similar to [std::is_nothrow_invocable](https://en.cppreference.com/w/cpp/types/is_invocable), but in a fashion similar to `csl::wf::mp::is_invocable`.

```cpp
struct F {
    template <typename ...>
    void operator()() const & {}
    void operator()() & noexcept {}
};

using namespace csl::wf;

static_assert(not mp::is_invocable<F&, int>);
static_assert(not mp::is_nothrow_invocable<F&, int>);

static_assert(mp::is_invocable<const F&>);
static_assert(mp::is_invocable<const F&, ttps<>>);
static_assert(not mp::is_nothrow_invocable<const F&>);

static_assert(mp::is_invocable<F&>);
static_assert(mp::is_invocable<F&, ttps<>>);
static_assert(mp::is_nothrow_invocable<F&>);
static_assert(mp::is_nothrow_invocable<F&, ttps<>>);
```

### is_invocable_r

```cpp
is_invocable_r<R, F, [ttps<...>,] args_types...>
```

| parameter | description |
| --------- | ----------- |
| `R`       | Type that the invocation result's value must be convertible to |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | template-type-parameters |
| `args_types` | Parameters types.<br>Must be complete types usable in an unevaluated context. |

---

Similar to [std::is_invocable_r]([std::is_invocable_r](https://en.cppreference.com/w/cpp/types/is_invocable)), but with an additional non-mandatory (possibly cv-ref-qualified) `ttps<...>` parameter to pass template-type-parameters. 
Determines whether 
- `F` can be invoked with non-mandatory template-type-parameter `ttps<...>`
- and arguments `args_types...`, to yield a result that is convertible to `R`

In a nutshell, 
- if `ttps<...>` is missing, or its template-type-parameters sequence is empty,  
then the behavior is strictly identical to a regular call to `std::is_invocable_r`.  
- if `ttps<...>` template-type-parameters **is not** empty,  
  then this will check if the type of the following expression is convertible to `R` :

```cpp
std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...)
```

---

Like its STL counterpart, if `R` is `void`, then the result can be any type.  

```cpp
auto func = []<typename ... Ts>(auto && value){ return std::common_type_t<decltype(value), Ts...>(); }
using F = decltype(func);

static_assert(csl::wf::mp::is_invocable_r_v<
  int,              // return type must be convertible to `int`
  F,                // functor type
  ttps<char, int>,  // ttps
  bool              // args
>);

static_assert(csl::wf::mp::is_invocable_r_v<
  int,              // return type must be convertible to `int`
  F,                // functor type
                    // no ttps
  bool              // args
>);

```

### is_nothrow_invocable_r

```cpp
is_nothrow_invocable_r<F, [ttps<...>,] args_types...>
```

| parameter | description |
| --------- | ----------- |
| `R`       | Type that the invocation result's value must be convertible to |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | template-type-parameters |
| `args_types` | Parameters types.<br>Must be complete types usable in an unevaluated context. |

---

Same as [is_invocable_r](#is_invocable_r), but with an additional non-mandatory (possibly cv-ref-qualified) `ttps<...>` parameter to pass template-type-parameters. 
Determines whether 
- `F` can be invoked with non-mandatory template-type-parameter `ttps<...>`
- and arguments `args_types...`, to yield a result that is convertible to `R`
- and that call is known **not** to throw any exception (`noexcept`).

Like its STL counterpart, if `R` is `void`, then the result can be any type.  
The conversion of the parameters and the call itself still has to be known **not** to throw any exceptions.

In a nutshell, 
- if `ttps<...>` is missing, or its template-type-parameters sequence is empty,  
then the behavior is strictly identical to a regular call to `std::is_invocable_r`.  
- if `ttps<...>` template-type-parameters **is not** empty,  
  then this will check if the type of the following expression is convertible to `R` :

```cpp
struct A{}; struct B{}; struct C{};
struct functor {
  auto operator()()    const          -> A { return {} };
  auto operator()(int) const noexcept -> B { return {} };

  template <typename T>
  auto operator()() const noexcept(std::same_as<T, C>) -> C { return {} };
};

// invocation is not noexcept
static_assert(not csl::wf::mp::is_nothrow_invocable_r<
  A,
  const functor
>);
static_assert(not csl::wf::mp::is_nothrow_invocable_r<
  A,
  const functor,
  ttps<>  // can be cvref-qualified
>);

// invocation's result type is not convertible to `A` (returns B)
static_assert(not csl::wf::mp::is_nothrow_invocable_r<
  A,
  const functor,
  int,
>);
// ok
static_assert(csl::wf::mp::is_nothrow_invocable_r<
  B,
  const functor,
  int,
>);
// same as before
static_assert(csl::wf::mp::is_nothrow_invocable_r<
  B,
  const functor,
  ttps<>
  int,
>);

// ok
static_assert(csl::wf::mp::is_nothrow_invocable_r<
  C,
  const functor,
  ttps<C>
>);
// not noexcept because of `noexcept(std::same_as<T, C>)`, where T = A
static_assert(not csl::wf::mp::is_nothrow_invocable_r<
  C,
  const functor,
  ttps<A>
>);

```

### invoke_result

```cpp
invoke_result<F, [ttps<...>,] args_types...>
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `args_types` | Parameters types.<br>Must be complete types usable in an unevaluated context. |

| member-type | description |
| ----------- | ----------- |
| type        | deduced type of an invoke expression |

---

Deduce the return type yield by an invoke expression, at compile time.

Similar to its STL counterpat [std::invoke_result](https://en.cppreference.com/w/cpp/types/result_of), or [std::result_of](https://en.cppreference.com/w/cpp/types/result_of) for previous standards, but with an additiona non-mandatory template-parameter-type which is a possibly cvref-qualified `ttps<...>`, meant to pass template-type-parameters to the functors in order to build the invoke expression.

If the invoke expression is not valid, then the member-type `type` is not defined.

---

Example :

```cpp
{
  auto func = [](char) -> int { return {}; };
  using F = decltype(func);

  static_assert(std::is_same_v<
    int,
    invoke_result<F, char>
  >);
}
{
  auto func = []<typename...>(char) -> int { return {}; };
  using F = decltype(func);

  static_assert(std::is_same_v<
    int,
    invoke_result<F, ttps<>, char>
  >);
  static_assert(std::is_same_v<
    int,
    invoke_result<F, ttps<double, float>, char>
  >);
}
```

### is_applyable

```cpp
is_applyable<F, [ttps<...>,] tuple_type>
is_applyable<F, tuple_type<[ttps<...>,] ...> >
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `tuple_type` | Tuple-like type, containing invocation parameters.<br>Must meet the `csl::wf::concepts::tuple_interface` concept requirements |

---

Similar to [is_invocable](#is_invocable), but detects if the callable object F is invocable using a tuple of argument.

Those can be passed in two different way, either as :

- Second non-mandatory template-type-parameter,  
  `is_applyable<F, [ttps<...>,] tuple_type>`
- First template-type-parameter of the tuple_type itself,  
  `is_applyable<F, tuple_type<ttps<...>, ...> >`

---

Example

```cpp
{
  auto func = [](A, B){};
  using F = decltype(func);
  static_assert(mp::is_applyable_v<F,         std::tuple<A, B>>);
  static_assert(mp::is_applyable_v<F, ttps<>, std::tuple<A, B>>);
}
{
  auto func = []<typename>(A, B){};
  using F = decltype(func);
  static_assert(mp::is_applyable_v<F, ttps<int>, std::tuple<A, B>>);
}
```

### is_nothrow_applyable

```cpp
is_nothrow_applyable<F, [ttps<...>,] tuple_type>
is_nothrow_applyable<F, tuple_type<[ttps<...>,] ...> >
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `tuple_type` | Tuple-like type, containing invocation parameters.<br>Must meet the `csl::wf::concepts::tuple_interface` concept requirements |

---

Similar to [is_nothrow_invocable](#is_nothrow_invocable), but detects if :
- the callable object F is invocable using a tuple of argument
- **and** is not known to throw any exception

---

Example

```cpp
{
  struct F {
    void operator()(A, B){} // 1
    template <typename>
    void operator()(A, B) noexcept {} // 2
  };

  static_assert(not mp::is_applyable_v<F,         std::tuple<A, B>>);     // 1
  static_assert(not mp::is_applyable_v<F, ttps<>, std::tuple<A, B>>);     // 1
  static_assert(    mp::is_applyable_v<F, ttps<int>, std::tuple<A, B>>);  // 2
}
```

### is_applyable_before

```cpp
// definitions
is_applyable_before<typename F, typename...>;
// specialization
is_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...>;
is_applyable_before<F, tuple_type, func_args_t...>;
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `tuple_type` | Tuple-like type, containing invocation parameters.<br>Must meet the `csl::wf::concepts::tuple_interface` concept requirements |
| `func_args_t`| Additional template-type-parameters.<br>Expanded **after** `tuple_type` |

---

Same as [is_applyable](#is_applyable), but provide the opportunity for the user to add extra parameters to the underlying functor invocation.
Nb : the parameters contained in the tuple-like type `tuple_type` are expand **before** `func_args_t` in the signature.

Internally use `is_invocable` to detect if such expression is true :
- ```cpp
  is_invocable_v<F, ttps<f_ts...>, decltype(std::get<indexes>(std::declval<tuple_type>()))..., func_args_t...>;
  ```

---

Example

```cpp
auto f = [](B, A){};
using F = decltype(f);

static_assert(is_applyable_before_v<F, std::tuple<>, B, A>);
static_assert(is_applyable_before_v<F, std::tuple<B>, A>);
static_assert(is_applyable_before_v<F, std::tuple<B, A>>);

static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>, B, A>);
static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B>, A>);
static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B, A>>);
```

### is_nothrow_applyable_before

Same as [is_applyable_before](#is_applyable_before), but internally use [is_nothrow_invocable](#is_nothrow_invocable) instead of [is_invocable](#is_invocable),  
so the evaluated invocation expression must NOT be known to throw.

---

Example

```cpp
auto f = []<typename T>(T) noexcept {};
using F = decltype(f);

static_assert(is_applyable_before_v<F, std::tuple<>, B>);
static_assert(is_applyable_before_v<F, std::tuple<B>>);
static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>, B>);
static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B>>);

static_assert(is_nothrow_applyable_before_v<F, std::tuple<>, B>);
static_assert(is_nothrow_applyable_before_v<F, std::tuple<B>>);
static_assert(is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>, B>);
static_assert(is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B>>);
```

### is_applyable_after

```cpp
// definitions
is_applyable_after<typename F, typename...>;
// specialization
is_applyable_after<F, ttps<f_ts...>, tuple_type, func_args_t...>;
is_applyable_after<F, tuple_type, func_args_t...>;
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `tuple_type` | Tuple-like type, containing invocation parameters.<br>Must meet the `csl::wf::concepts::tuple_interface` concept requirements |
| `func_args_t`| Additional template-type-parameters.<br>Expanded **before** `tuple_type` |

Same as [is_applyable_before](#is_applyable_before), but parameters contained in the tuple-like type are expand **after** `func_args_t`.

Internally use `is_invocable` to detect if such expression is true :

- ```cpp
  is_invocable_v<F, ttps<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
  ```

---

Example

```cpp
auto f = [](A, B){};
using F = decltype(f);

static_assert(is_applyable_after_v<F, std::tuple<>, A, B>);
static_assert(is_applyable_after_v<F, std::tuple<B>, A>);
static_assert(is_applyable_after_v<F, std::tuple<A, B>>);

static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>, A, B>);
static_assert(is_applyable_after_v<F, ttps<>, std::tuple<B>, A>);
static_assert(is_applyable_after_v<F, ttps<>, std::tuple<A, B>>);
```

### is_nothrow_applyable_after

Same as [is_applyable_after](#is_applyable_after), but internally use [is_nothrow_invocable](#is_nothrow_invocable) instead of [is_invocable](#is_invocable),  
so the evaluated invocation expression must NOT be known to throw.

---

Example

```cpp
auto f = [](A, B) noexcept {};
using F = decltype(f);

static_assert(is_applyable_after_v<F, std::tuple<>, A, B>);
static_assert(is_applyable_after_v<F, std::tuple<B>, A>);
static_assert(is_applyable_after_v<F, std::tuple<A, B>>);

static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>, A, B>);
static_assert(is_applyable_after_v<F, ttps<>, std::tuple<B>, A>);
static_assert(is_applyable_after_v<F, ttps<>, std::tuple<A, B>>);

static_assert(is_nothrow_applyable_after_v<F, std::tuple<>, A, B>);
static_assert(is_nothrow_applyable_after_v<F, std::tuple<B>, A>);
static_assert(is_nothrow_applyable_after_v<F, std::tuple<A, B>>);

static_assert(is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>, A, B>);
static_assert(is_nothrow_applyable_after_v<F, ttps<>, std::tuple<B>, A>);
static_assert(is_nothrow_applyable_after_v<F, ttps<>, std::tuple<A, B>>);
```

### apply_result

```cpp
// definition
struct apply_result<typename F, typename...>;
// specializations
struct apply_result<F, tuple_type>; // 1
struct apply_result<F, ttps_type, tuple_type>; // 2
struct apply_result<F, ttps<ttps_args...>, concepts::tupleinterface_not_starting_with_ttps>; // 3
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>`  | Template-type-parameters |
| `tuple_type` | Tuple-like type, containing invocation parameters.<br>Must meet the `csl::wf::concepts::tuple_interface` concept requirements |

---

Deduces the type yield by an [apply](#apply) expression, in a similar way in which [invoke_result](#invoke_result) deduce the type of an [invoke](#invoke) expression.

Non-mandatory additional parameter `ttps<...>` can be pass to represents template-type-paramters for the underlying functor value call, either as an :

- Additional `ttps<...>` template-parameter, using (2) specialization.
- Additional first tuple element of `tuple_type`, using (3) specialization.

Note that both way to pass `ttps<...>` can't be used at the same time.

---

Example

```cpp
struct A{}; struct B{};
auto func = []<typename ... Ts>(A, B) -> std::common_type_t<Ts...> { return {}; };
using F = decltype(func);

static_assert(std::is_same_v<
  int,
  apply_result_t<F, std::tuple<A, B>>
>); // error : invalid use of incomplete type std::common_type<Ts...> where [ Ts = ] (empty)

static_assert(std::is_same_v<
  int,
  apply_result_t<F, ttps<char, bool, int>, std::tuple<A, B>>
>);
static_assert(std::is_same_v<
  int,
  apply_result_t<F, std::tuple<ttps<char, bool, int>, A, B>>
>);
```

### is_invocable_with

```cpp
// definition
struct is_invocable_with<typename F, typename ...>;
// specializations
template <typename F, typename ... ttps_args, typename ... args_types>
struct is_invocable_with<F, ttps<ttps_args...>, args<args_types...>>
```

| parameter | description |
| --------- | ----------- |
| `F`       | A type, most likely a functor |
| `ttps<...>` | Template-type-parameters |
| `args<...>` | Parameters |

---

Similar to [is_invocable](#is_invocable)/[is_applyable](#is_applyable), but wraps both template-type-parameters and parameters respectively as `ttps` and `args` template-type-parameters.  

More convenient for pack_traits - *like filtering* - applications

Less restrictive than `is_applyable`, as `args<...>` is **NOT** requiered to match `TupleInterface`.  
Also, easier to handle types that does not fit in `std::tuple`, `array` and `paires`, like `void`.

---

Example :

```cpp
struct F {
  template <typename ...>
  void operator()(){}
  template <typename T>
  void operator()(int) const noexcept {}
};

static_assert(invocable_with<F, ttps<>, args<>>);
static_assert(invocable_with<F, ttps<char, bool>, args<>>);
static_assert(invocable_with<const F, ttps<char>, args<int>>);
```

### is_nothrow_invocable_with

Similar to [is_invocable_with](#is_invocable_with), but the underlying detected call must not be known to throw any exception.

---

Example :

```cpp
struct F {
  template <typename ...>
  void operator()(){}
  template <typename T>
  void operator()(int) const noexcept {}
};

static_assert(not nothrow_invocable_with<F, ttps<>, args<>>);
static_assert(nothrow_invocable_with<const F, ttps<char>, args<int>>);
```

## bind_front

Factory for `front_binder`.  
Like `std::bind_front`, preserve cvref-qualifiers correctness.

Signature :

```cpp
template <typename ... ttps_bounded_args_t, typename F, typename ... args_t>
constexpr auto bind_front(F&& f, args_t && ... args);
```

### front_bindable (concept)

### front_binder (type)

## invocation policies

### csl::wf::invoke_policy::allow_discard

### csl::wf::invoke_policy::nodiscard

## chain

### `mp::chain_trait<typename ... Fs>`

### chain_invoke

### chain_invoke_nodiscard

### chain_invocable (concept)

### route (type)

#### get<index>(instance_of<route>)

## overload

## repeater

## function_ref<T>

Wraps a reference to a value which might be a functor,  
in a similar fashion to [std::reference_wrapper](https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper).  
However, in opposition to `std::reference_wrapper::operator()`, `function_ref<T>::operator()`-s are conditionaly generated and internally call `csl::wf::invoke` with a similar signature (see description hereunder).

In opposition to [`function_view<T>`](#function_viewt), a `function_ref<T>` value si copyable and reassignable using its `operator=` or `swap` member-functions.

> This is somehow similar to the [std::function_ref proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0792r5.html),  
> but designed & implemented according to `csl::wf` philosophy.  
> See [](#philosophy--design-choices) and [](#invocation-utilities) sections.

To avoid any dangling reference, a `function_ref<T>` value's lifetime must be greater than the underlying data it refers to.

**Member types**

| type | definition |
| ---- | ---------- |
| type | T          |

**Member functions - constructors**

| name | description |
| ---- | ----------- |
| `constexpr explicit function_ref(auto && value) noexcept` | by-value constructor<br>If `value` is a pointer, then it can't be equal to `nullptr` | 
| `constexpr function_ref(const function_ref & other) noexcept = default;` | copy-constructor |
| `constexpr function_ref(function_ref &&) noexcept = default;` | move-constructor |
| `constexpr function_ref() = delete;` | default constructor |
| `constexpr ~function_ref() = default;` | default destructor |

**Member functions - assign**

| name | description |
| ---- | ----------- |
| `constexpr function_ref & operator=(function_ref &&) noexcept = default;` | move-assign |
| `constexpr function_ref & operator=(const function_ref &) noexcept = default;` | copy-assign |
| `constexpr void swap(function_ref & other) noexcept` | swap | 

**Member functions - accessors**

| name | description |
| ---- | ----------- |
| `constexpr explicit operator /*cv-qualifier*/ type&() /*cv-qualifier*/ noexcept` | Underlying value |
| `constexpr /*cv-qualifier*/ F& get() /*cv-qualifier*/ noexcept` | Underlying value |

**Member functions - operations**

> The following `operator()` overload are conditionaly generated.

| name | description |
| ---- | ----------- |
| `template <typename ... ttps_args>`<br>`constexpr decltype(auto) operator()(auto && ... args) /*cv-ref-qualifiers*/` | Forward parameters to [`csl::wf::invoke`](#invoke), and returns the invocation's result.  |

**Non-member functions**

| name | description |
| ---- | ----------- |
| `template <typename F>`<br>`constexpr void swap(function_ref<F> & lhs, function_ref<F> & rhs)` | swap |

## function_view<T>

Same as [`function_ref<T>`](#function_reft), but **not** reassignable.

## eDSL

### _times (literals)

### factories

#### make_continuation

#### make_condition

#### make_repetition

### operators

#### `operator|` (pipe operator)

##### F1 | F2

##### F | ref

Returns a value of type [`function_ref<T>`](#function_reft).

##### F | cref

Returns a value of type [`function_ref<const T>`](#function_reft), where type is const-qualified.

##### F | view

Returns a value of type [`function_view<T>`](#function_viewt).

#### `operator>>=` (right-shift-assign operator)

#### `operator*` (star operator)
