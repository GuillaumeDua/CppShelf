# Documentation : csl::wf

The goal of `csl::wf` is to offer convenient ways to manipulate functors & callable values in general.

This library is divided in three parts :

- Invocation [**functions**](#invocation-utilities) *(`invoke`, `apply`, etc.)*,  
  and [**type-traits**](#invocation-traits) *(`is_invocable`, `is_applyable`, `is_nothrow_invocable_r`, etc.)*
- Functors [**types**]() with specific purpose *(front_binder, overload, repeater, etc.)*
- An [**eDSL**](#edsl) to create complexe workflow easily

---

- [Documentation : csl::wf](#documentation--cslwf)
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
      - [`operator>>=` (right-shift-assign operator)](#operator-right-shift-assign-operator)
      - [`operator*` (star operator)](#operator-star-operator)

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

Available either as type-traits, or constexpr template-variable.

### is_invocable

### is_nothrow_invocable

### is_invocable_r

### is_nothrow_invocable_r

### invoke_result

### is_applyable

### is_nothrow_applyable

### is_applyable_before

### is_nothrow_applyable_before

### is_applyable_after

### is_nothrow_applyable_after

### apply_result

### is_invocable_with

### is_nothrow_invocable_with

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

Wraps a reference to a functor in a copyable & assignable object,  
in a similar fashion to [std::reference_wrapper](https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper).

However, in opposition to `std::reference_wrapper::operator()`, `function_ref<T>::operator()`-s are conditionaly generated and internally call `csl::wf::invoke` (see description hereunder).

Member types

| type | definition |
| ---- | ---------- |
| value_type | T    |

Member functions

| name | description |
| ---- | ----------- |


## function_view<T>

## eDSL

### _times (literals)

### factories

#### make_continuation

#### make_condition

#### make_repetition

### operators

#### `operator|` (pipe operator)

#### `operator>>=` (right-shift-assign operator)

#### `operator*` (star operator)
