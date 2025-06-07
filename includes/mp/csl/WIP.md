# WIP

- storage: explicit, noexcept, etc.
- conditionally enable implicit conversions
  - `tuple_storage<indexes, Us...>`

- tuple_element: no storage, just eq to `std::tuple_element<I,T>`
- tuple_element_value: `decltype( get<I>(std::declval<T>()))`
  - which (if any) cvref-qualifier for T ? -> T&
- namespace `csl::mp::tuple` ?
  - type ?
  - size
  - element
  - etc.
