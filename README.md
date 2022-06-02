<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/Shelf_icons_created_by_juicy_fish_Flaticon.png?raw=true" alt="" align="left" width="128" height="128" style="Padding: 0px 10px 0px 0px"/>

# C++ Shelf

[![cmake-Ubuntu-clang](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-clang.yml/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-clang.yml)
[![cmake-Ubuntu-gcc](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-gcc.yml/badge.svg)](https://github.com/GuillaumeDua/CppShelf/actions/workflows/cmake-Ubuntu-gcc.yml)
[![GitHub license](https://img.shields.io/github/license/GuillaumeDua/CppShelf)](https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE)  
![GitHub commit activity](https://img.shields.io/github/commit-activity/y/GuillaumeDua/CppShelf)
[![GitHub issues](https://img.shields.io/github/issues/GuillaumeDua/CppShelf)](https://github.com/GuillaumeDua/CppShelf/issues)

---

Collection of - ***S**ingle-**He**ader, header-only, C++ **l**ibraries **f**iles*.  
Personal incubator for various library ideas and experiences, and perhaps a hint of serendipity.

The libraries here could, once mature enough, become self-contained on a separate repository.

---

<img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/Shelf_icons_created_by_juicy_fish_Flaticon.png?raw=true" alt="" align="left" width="26" height="26" style="Padding: 5px"/>

## Components - nested librairies

### 🌊 [wf](https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/markdown/wf.md)

Generic **eDSL** builder - *as a route pattern* - to best translate **workflow**/use-cases in C++

See project's
[documentation](https://guillaumedua.github.io/CppShelf/output/wf/), 
[dashboard](https://github.com/GuillaumeDua/CppShelf/projects/1)

### 🧙‍♂️ [mp](https://github.com/GuillaumeDua/CppShelf/blob/main/includes/mp/csl/mp.hpp)

A collection of **C++ TMP** *(C++ template-metaprogramming)* utilities

Project's [dashboard here](https://github.com/GuillaumeDua/CppShelf/projects/2)

---

<img src="https://raw.githubusercontent.com/isocpp/logos/master/cpp_logo.svg" alt="" align="left" width="32" height="32" style="Padding: 5px"/>

## Supported C++ standard

<br>

Currently, **C++20**.

Backward compatibility with **C++17** is mostly doable, but requires additional effort and is not something planned for now.  

Many implementations details will change when **C++23** is [officially supported by compiliers](https://en.cppreference.com/w/cpp/compiler_support).  
👉 See opened issue [tagged with C++23](https://github.com/GuillaumeDua/CppShelf/issues?q=is%3Aissue+is%3Aopen+label%3AC%2B%2B23).

## Supported compilers

| Compiler       | Version       | Reason |
| -------------- | ------------- | ------ |
| g++            | >= 11.1.0     | C++20  |
| clang++        | >= 13.0.1     | C++20<br>Decent support of [p0315r4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0315r4.pdf) |
| Visual Studio  | *TBD*         | *TBD*  |

---

This Readme.md 's ressources

- [Shelf icons created by juicy_fish - Flaticon](https://www.flaticon.com/free-icons/shelf)
