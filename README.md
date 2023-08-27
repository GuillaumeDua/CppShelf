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

# C++ Shelf

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
[dashboard](https://github.com/GuillaumeDua/CppShelf/projects/4)

### 🌊 [wf](https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/markdown/wf.md)

Generic **eDSL** builder - *as a route pattern* - to best translate **workflow**/use-cases in C++

See project's
[documentation](https://guillaumedua.github.io/CppShelf/output/wf/), 
[dashboard](https://github.com/GuillaumeDua/CppShelf/projects/1)

### 🧙‍♂️ [mp](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/mp/csl/mp.hpp)

A collection of **C++ TMP** *(C++ template-metaprogramming)* utilities

Project's [dashboard](https://github.com/GuillaumeDua/CppShelf/projects/2)

### 🔬 [functional](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/functional/csl/functional.hpp)

Function & overload traits

See project's [live demo on godbolt.org](https://godbolt.org/z/ssEW9ferd)

### 🔎 [typeinfo](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/typeinfo/csl/typeinfo.hpp)

Compile-time demangler, for types & values.  
Inconsistent: for educational and debugging purpose only.

See project's [live demo on godbolt.org](https://godbolt.org/z/TsEWf5bGa)

### ☔ [ensure](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/ensure/csl/ensure.hpp)

Interface-safety related components, including a configurable **strong-type** implementation.

---

## Getting Started - CMake

TODO: fetch_content example in README, for 1, N components

---

<img src="https://raw.githubusercontent.com/isocpp/logos/master/cpp_logo.svg" alt="" align="left" width="25" height="25"/>

## Supported C++ standard

| Library       | C++17 | C++20 | C++23 |
| ------------- | :---: | :---: | :---- |
| 📦 ag         | ❌   | ✅   | -     |
| 🌊 wf         | ❌   | ✅   | *planned*<br>[P1985 - Universal template parameters](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r1.pdf)<br>[P0847 - Deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0847r5.html) |
| 🧙‍♂️ mp         | ❌   | ✅   | -     |
| 🔬 functional | ❌   | ✅   | -     |
| 🔎 typeinfo   | ❌   | ✅   | -     |
| ☔ ensure     | ✅   | ✅   | -     |

> 💡 Backward compatibility with **C++17** requires additional effort and IS NOT a priority for now, beside specific requests.

Implementations details might change when **C++23** is [fully supported by compiliers](https://en.cppreference.com/w/cpp/compiler_support).  
👉 See opened issue [tagged with C++23](https://github.com/GuillaumeDua/CppShelf/issues?q=is%3Aissue+is%3Aopen+label%3AC%2B%2B23).

## Supported compilers

| Compiler       | Min. version  | Recommended | Reason |
| -------------- | ------------- | ----------- | ------ |
| `g++`          | >= 11.1.0     | >= 13.2     | C++20  |
| `clang++`      | >= 13.0.1     | >= 16.0.6   | C++20<br>Decent support of [p0315r4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0315r4.pdf) |
| `msvc-cl`      | *TBD*         | *TBD*       | -      |

---

This Readme.md 's ressources

- [Shelf icons created by juicy_fish - Flaticon](https://www.flaticon.com/free-icons/shelf)
