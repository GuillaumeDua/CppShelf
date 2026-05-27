# C++ Shelf

Collection of single-header, header-only C++ libraries.  
[Source on GitHub](https://github.com/GuillaumeDua/CppShelf), MIT License

---

## Libraries

![dependency graph](https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/doc/images/csl.svg)

| Library                                                                              | Description                                 | API docs                | C++17 | C++20 |  C++23  |
| ------------------------------------------------------------------------------------ | ------------------------------------------- | :---------------------- | :---: | :---: | :-----: |
| 📦 [ag](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/ag)                 | Tuple-like interface for aggregates         | @subpage lib_ag         |  ❌   |  ✅   |   ✅    |
| 🌊 [wf](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/wf)                 | Generic eDSL builder / workflow pattern     | @subpage lib_wf         |  ❌   |  ✅   | planned |
| 🧙 [mp](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/mp)                 | C++ template-metaprogramming utilities      | @subpage lib_mp         |  ❌   |  ✅   |   ✅    |
| 🔬 [functional](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/functional) | Function & overload traits                  | @subpage lib_functional |  ❌   |  ✅   |   ✅    |
| 🔎 [typeinfo](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/typeinfo)     | Compile-time demangler for types & values   | @subpage lib_typeinfo   |  ❌   |  ✅   |   ✅    |
| ☔ [ensure](https://github.com/GuillaumeDua/CppShelf/tree/main/libs/ensure)          | Interface-safety, configurable strong types | @subpage lib_ensure     |  ✅   |  ✅   |   ✅    |

---

## CMake modules

@subpage lib_cmake

| Module                       | Description                                                                                                     |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `csl/cache_entry_conversion` | Converts a `CACHE STRING` entry to the integer index of its current value within the entry's `STRINGS` property |
| `csl/get_cpm`                | Downloads and includes [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)                                      |
| `csl/graphviz`               | Custom target that runs `cmake --graphviz` and renders the result with `dot` and `svg`                          |
| `csl/list_subdirectories`    | Populates a variable with all immediate subdirectories of a given path                                          |
| `csl/print_aligned`          | Aligned-column status output for CMake configure-time logging                                                   |
