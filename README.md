# C++ Shelf

<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/Shelf_icons_created_by_juicy_fish_Flaticon.png?raw=true" alt="" align="left" height="150" style="Padding: 0px 10px 0px 0px"/>

<div style="position: absolute; top: 0; right: 0;">
<a href="https://github.com/GuillaumeDua/CppShelf">
<img loading="lazy" width="149" height="149" src="https://github.blog/wp-content/uploads/2008/12/forkme_right_red_aa0000.png?resize=149%2C149" class="attachment-full size-full" alt="Fork me on GitHub" data-recalc-dims="1" align="right"
style="position: relative; top: 0; right: 0; z-index: 1;">
</a>

<a href="https://guillaumedua.github.io/CppShelf/">
<img loading="lazy" src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/gh-pages-logo.png?raw=true" alt="Check documentation on GitHub-pages" align="right" class="attachment-full size-full" height="40"
style="position: absolute; top: 0; right: 0; z-index: 3;"
>
</a>
</div>

[![cmake-Ubuntu-clang](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-clang.yml/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-clang.yml)
[![cmake-Ubuntu-gcc](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-gcc.yml/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-gcc.yml)  
[![pages-build-deployment](https://github.com/GuillaumeDua/CppShelf/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/pages/pages-build-deployment)
[![Deploy-doc-to-GitHub-Pages](https://github.com/GuillaumeDua/CppShelf/actions/workflows/docs_to_gh_pages.yml/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/docs_to_gh_pages.yml)  
[![GitHub license](https://img.shields.io/github/license/GuillaumeDua/CppShelf)](https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE)
![GitHub top language](https://img.shields.io/github/languages/top/GuillaumeDua/CppShelf)
[![GitHub issues](https://img.shields.io/github/issues/GuillaumeDua/CppShelf)](https://github.com/GuillaumeDua/CppShelf/issues)
![GitHub commit activity](https://img.shields.io/github/commit-activity/y/GuillaumeDua/CppShelf)

---

Collection of - ***S**ingle-**He**ader, header-only, C++ **l**ibraries **f**iles*.  
Personal incubator for various library ideas and experiences, and perhaps a hint of serendipity.

The libraries here could, once mature enough, become self-contained in a separated repository.

---

<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/Shelf_icons_created_by_juicy_fish_Flaticon.png?raw=true" alt="" align="left" width="25" height="25" style="Padding: 5px"/>

## Components - nested librairies

### 📦 [ag](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/ag/csl/ag.hpp)

A **tuple-like** interface for **aggregates** type.

Project's
[documentation](https://guillaumedua.github.io/CppShelf/output/ag/),
[dashboard](https://github.com/users/GuillaumeDua/projects/2)

### 🌊 [wf](https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/markdown/wf.md)

Generic **eDSL** builder - *as a route pattern* - to best translate **workflow**/use-cases in C++

See project's
[documentation](https://guillaumedua.github.io/CppShelf/output/wf/),
[dashboard](https://github.com/GuillaumeDua/CppShelf/projects/1)

### 🧙‍♂️ [mp](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/mp/csl/mp.hpp)

A collection of **C++ TMP** *(C++ template-metaprogramming)* utilities

Project's [dashboard](https://github.com/users/GuillaumeDua/projects/3)

### 🔬 [functional](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/functional/csl/functional.hpp)

Function & overload traits

See project's
[live demo on godbolt.org](https://godbolt.org/z/Gd7fMKK8e),
[dashboard](https://github.com/users/GuillaumeDua/projects/5)

### 🔎 [typeinfo](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/typeinfo/csl/typeinfo.hpp)

Compile-time demangler, for types & values.  
Inconsistent: for educational and debugging purpose only.

See project's
[live demo on godbolt.org](https://godbolt.org/z/TsEWf5bGa),
[dashboard](https://github.com/users/GuillaumeDua/projects/7)

### ☔ [ensure](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/ensure/csl/ensure.hpp)

Interface-safety related components, including a configurable **strong-type** implementation.

See project's
[dashboard](https://github.com/users/GuillaumeDua/projects/4)

---

<img src="https://raw.githubusercontent.com/isocpp/logos/master/cpp_logo.svg" alt="" align="left" width="25" height="25" style="Padding: 5px"/>

## Supported C++ standard

| Library       | C++17 | C++20 | C++23 |
| ------------- | :---: | :---: | :---- |
| 📦 ag         | ❌   | ✅   | ✅    |
| 🌊 wf         | ❌   | ✅   | *planned*<br>[P1985 - Universal template parameters](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r1.pdf)<br>[P0847 - Deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0847r5.html) |
| 🧙‍♂️ mp         | ❌   | ✅   | -     |
| 🔬 functional | ❌   | ✅   | ✅    |
| 🔎 typeinfo   | ❌   | ✅   | ✅    |
| ☔ ensure     | ✅   | ✅   | ✅    |

> 💡 Note that implementation details might changes from a version to another,  
> and in some cases, the source file itself.
>
> For instance, we currently have:
> `<csl/ensure.hpp>` which either includes `<csl/cxx_17/ensure.hpp>`, or `<csl/cxx_20/ensure.hpp>` when C++20 or greater is available.

### About C++17 support

💡 Backward compatibility with **C++17** requires additional effort and **IS NOT a priority** for now, beside specific requests.  

### About C++23 support

⚠️ Implementations details might change once specific **C++23** [features are supported](https://en.cppreference.com/w/cpp/compiler_support) by targeted compilers.  
👉 See opened issue [tagged with C++23](https://github.com/GuillaumeDua/CppShelf/issues?q=is%3Aissue+is%3Aopen+label%3AC%2B%2B23).

## Supported compilers

| Compiler       | Min. version  | Recommended | Reason |
| -------------- | ------------- | ----------- | ------ |
| **`g++`**      | >= 11.1.0     | >= 13.2     | C++20  |
| **`clang++`**  | >= 13.0.1     | >= 16.0.6   | C++20<br>Decent support of [P0315r4 lambdas in unevaluated contexts](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0315r4.pdf) |
| **`msvc-cl`**  | *TBD*         | *TBD*       | -      |

---

## Getting started

### Getting started - using CMake

👉 This is the recommended way to integrate `csl` (or a subset of its components) to a project.

- [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)

    ```cmake
    include(FetchContent)

    # Forces csl a options ...
    # example: set(CSL_TEST_ALL OFF CACHE INTERNAL "")

    FetchContent_Declare(
        csl
        GIT_REPOSITORY https://github.com/GuillaumeDua/CppShelf.git
    )
    FetchContent_MakeAvailable(csl)
    ```

- [ExternalProject_Add](https://cmake.org/cmake/help/latest/module/ExternalProject.html)

#### CMake - options

> 💡Each cache entry is structured as `CSL_<WHAT>_<ALL|NAME>`.

General options:

| Option              | Type | Default | Description |
| ------------------- | ---- | ------- | ----------- |
| `CSL_BUILD_ALL`     | bool | ON      | enable/disable all components **build**      |
| `CSL_INSTALL_ALL`   | bool | ON      | enable/disable all components **install**    |
| `CSL_TEST_ALL`      | bool | OFF     | enable/disable all components **tests**      |
| `CSL_EXAMPLE_ALL`   | bool | OFF     | enable/disable all components **examples**   |
| `CSL_BENCHMARK_ALL` | bool | OFF     | enable/disable all components **benchmarks** |

Components-specific options:

| Option syntax                      | Type | Default / dependent       | Description |
| ---------------------------------- | ---- | ------------------------- | ----------- |
| `CSL_BUILD_\<component_name\>`     | BOOL | `CSL_BUILD_ALL`           | enable/disable a specific component **build**   |
| `CSL_INSTALL_\<component_name\>`   | BOOL | `CSL_INSTALL_ALL`         | enable/disable a specific component **install** |
| `CSL_TEST_\<component_name\>`      | BOOL | `CSL_TEST_ALL`            | enable/disable a specific component **test**    |
| `CSL_EXAMPLE_\<component_name\>`   | BOOL | `CSL_EXAMPLE_ALL`         | enable/disable a specific component **example** |
| `CSL_BENCHMARK_\<component_name\>` | BOOL | `CSL_BENCHMARK_ALL`       | enable/disable a specific component **benchmark** |

For options related to a specific component, refer to its dedicated documentation.

### Getting started - header-only

Using the way you prefer, acquire a component/library file.

- `wget` shell command
- CMake [`file(DOWNLOAD <url> [<file>] [...])`](https://cmake.org/cmake/help/latest/command/file.html)
- etc.

Use it in your project using an `#include` preprocessor directive.  
Refer to the component's documention to check for configuration preprocessor constant.

Example:

```cpp
#define CSL_ENSURE__ENABLE_FMT_SUPPORT // enable `fmt` support, if available
#include <csl/ensure.hpp>

void func(){
    using meters = csl::ensure::strong_type<int, struct meter_tag>;
    fmt::print(meters{ 42 });
}
```

### In [Compiler Explorer](https://godbolt.org/)

See [demonstration here](https://godbolt.org/z/4aGfEbf8d).

```cpp
#include <iostream>

#include <https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/typeinfo/csl/typeinfo.hpp>
#include <https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/ag.hpp>

struct some_model {
    char c = 'A';
    int i = 42;
};

auto main() -> int {
    std::cout
        << csl::typeinfo::type_name_v<
            csl::ag::element_t<0, some_model> // char
           >
    ;
    return csl::ag::get<1>(some_model{}); // 42
}
```

Possible output:

```log
Program returned: 42
Program stdout
char
```

### Advanced examples

#### Aggregate: print type-value pairs

```cpp
#include <csl/ag.hpp>

template <csl::ag::concepts::aggregate type>
void print(const type & value){
    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((
            std::cout
                << csl::typeinfo::type_name_v<csl::ag::element_t<0, type>>
                << ": "
                << csl::ag::get<indexes>(value)
                << '\n'
           >
        ), ...);
    ;
    }(std::make_index_sequence<csl::ag::size_v<type>>{});
}

auto main() -> int { print(some_model{}); }
```

Possible output:

```log
char: 'A'
int: 42
```

#### All: pretty-printing

```cpp
#include <csl/typeinfo.hpp>
#include <csl/cxx20/ensure.hpp> // with CSL_ENSURE__ENABLE_FMT_SUPPORT enable from CMake cache
#include <csl/ag.hpp> // with CSL_AG__ENABLE_FMT_SUPPORT enable from CMake cache

struct A { char c = 'c'; };
using meters = csl::ensure::strong_type<int, struct meter_tag>;
struct B {
    meters i = 42;
    A a;
};
struct C {
    std::array<float> range = { .1F, .2F };
    B b;
    csl::mp::tuple<bool>{ true }
};

auto main() -> int {
    fmt::print("{}\n",              A{});
    fmt::print("\nindented:\n{}\n", A{} | csl::ag::io::indented);
}
```

Possible output:

```log
{ [ .1F, .2F ], { 42, { 'c' } } , ( true ) }

indented:
{
    [
        .1F,
        .2F
    ],
    {
        42,
        { 'c' }
    },
    (
        true
    )
}
```

---

## Misc

This Readme.md 's ressources

- [Shelf icons created by juicy_fish - Flaticon](https://www.flaticon.com/free-icons/shelf)
