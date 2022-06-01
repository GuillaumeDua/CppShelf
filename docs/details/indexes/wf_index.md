# 🌊 **csl::wf**

The goal of `csl::wf` is to offer convenient ways to manipulate functors & callable values in general.

This library is divided in three parts :

- Invocation [**functions**](#invocation-utilities) *(`invoke`, `apply`, etc.)*,  
  and [**type-traits**](#invocation-traits) *(`is_invocable`, `is_applyable`, `is_nothrow_invocable_r`, etc.)*
- Functors [**types**]() with specific purpose *(front_binder, overload, repeater, etc.)*
- An [**eDSL**](#edsl) to create complexe workflow easily

## Philosophy & design choices

The key idea of this library is to provide a convenient way to deal with functors which `operator()` are conditionaly-generated, have multiple overloads, or simply have template-parameters.

> As-is, only template-type parameters are supported.

Such design is in opposition to STL's `<functional>`, where types like `std::function` provides a type-erasure restrained by a unique function signature.

To allow multiples templates-parameters-pack in signatures, and represent for instance in one hand template-type-parameters and generic-parameters on the other,  
a type named `ttps<...>` is used to ease deduction.  
For some specific corner-cases, a type named `arg<...>` is also used to wrap `parameters`'s types in the same way.

It also provides a precise support to `operator()` cvref-qualifiers.
