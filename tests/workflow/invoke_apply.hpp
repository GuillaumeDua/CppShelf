#pragma once 

#include "../../workflow/workflow.hpp"

namespace test::functional::invocation {
    consteval void invocable() {
        using namespace workflow::functional;

        // f()
        {
            constexpr auto f = [](){};

            static_assert(requires{ invoke(f); });
            static_assert(requires{ invoke<>(f); });

            static_assert(not noexcept(invoke(f)));
            static_assert(not noexcept(invoke<>(f)));
        }
        {
            constexpr auto f = []() noexcept {};

            static_assert(requires{ invoke(f); });
            static_assert(requires{ invoke<>(f); });

            static_assert(noexcept(invoke(f)));
            static_assert(noexcept(invoke<>(f)));
        }
        // f(...)
        {
            constexpr auto f = [](int, bool){};

            static_assert(requires{ invoke(f, 42, true); });
            static_assert(requires{ invoke<>(f, 42, true); });

            static_assert(not noexcept(invoke(f, 42, true)));
            static_assert(not noexcept(invoke<>(f, 42, true)));
        }
        {
            constexpr auto f = [](int, bool) noexcept {};

            static_assert(requires{ invoke(f, 42, true); });
            static_assert(requires{ invoke<>(f, 42, true); });

            static_assert(noexcept(invoke(f, 42, true)));
            static_assert(noexcept(invoke<>(f, 42, true)));
        }
        // f<...>()
        {
            constexpr auto f = []<typename, typename>(){};

            static_assert(requires{ invoke<int, char>(f); });
            static_assert(not noexcept(invoke<int, char>(f)));
        }
        // f<T>(T)
        {
            constexpr auto f = []<typename T>(T){};

            static_assert(requires{ invoke(f, 42); });
            static_assert(requires{ invoke<>(f, 42); });
            static_assert(requires{ invoke<int>(f, 42); });

            static_assert(not noexcept(invoke(f, 42)));
            static_assert(not noexcept(invoke<>(f, 42)));
            static_assert(not noexcept(invoke<int>(f, 42)));
        }
        // f<t...>(auto...)
        {
            constexpr auto f = []<typename...>(auto...){};

            static_assert(requires{ invoke(f, 42); });
            static_assert(requires{ invoke<>(f, 42); });
            static_assert(requires{ invoke<int>(f, 42); });
            static_assert(requires{ invoke<int, char>(f, 42); });
            static_assert(requires{ invoke<int, char>(f, 42, true, 'a'); });
        }
    }
    consteval void applyable() {
        namespace ns = workflow::functional; // stl namespace clash https://godbolt.org/z/Kz9PjMs3P

        // f()
        {
            constexpr auto f = [](){};

            static_assert(requires{ ns::apply(f, std::tuple{}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{}); });
            static_assert(not noexcept(ns::apply(f, std::tuple{})));
            static_assert(not noexcept(ns::apply<>(f, std::tuple{})));
        }
        {
            constexpr auto f = []() noexcept {};

            static_assert(requires{ ns::apply(f, std::tuple{}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{}); });
            static_assert(noexcept(ns::apply(f, std::tuple{})));
            static_assert(noexcept(ns::apply<>(f, std::tuple{})));
        }
        {
            constexpr auto f = [](int, bool){};

            static_assert(requires{ ns::apply(f, std::tuple{42, true}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{42, true}); });
            static_assert(not noexcept(ns::apply(f, std::tuple{42, true})));
            static_assert(not noexcept(ns::apply<>(f, std::tuple{42, true})));
        }
        {
            constexpr auto f = [](int, bool) noexcept {};

            static_assert(requires{ ns::apply(f, std::tuple{42, true}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{42, true}); });
            static_assert(noexcept(ns::apply(f, std::tuple{42, true})));
            static_assert(noexcept(ns::apply<>(f, std::tuple{42, true})));
        }
        // f<...>()
        {
            constexpr auto f = []<typename, typename>(){};

            static_assert(requires{ ns::apply<char, int>(f, std::tuple{}); });
            static_assert(not noexcept(ns::apply<char, int>(f, std::tuple{})));
        }
        {
            constexpr auto f = []<typename, typename>() noexcept {};

            static_assert(requires{ ns::apply<char, int>(f, std::tuple{}); });
            static_assert(noexcept(ns::apply<char, int>(f, std::tuple{})));
        }
        // f<T>(T)
        {
            constexpr auto f = []<typename T>(T){};

            static_assert(requires{ ns::apply(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<char>(f, std::tuple{'a'}); });
            
            static_assert(not noexcept(ns::apply(f, std::tuple{'a'})));
            static_assert(not noexcept(ns::apply<>(f, std::tuple{'a'})));
            static_assert(not noexcept(ns::apply<char>(f, std::tuple{'a'})));
        }
        {
            constexpr auto f = []<typename T>(T) noexcept {};

            static_assert(requires{ ns::apply(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<char>(f, std::tuple{'a'}); });
            
            static_assert(noexcept(ns::apply(f, std::tuple{'a'})));
            static_assert(noexcept(ns::apply<>(f, std::tuple{'a'})));
            static_assert(noexcept(ns::apply<char>(f, std::tuple{'a'})));
        }
        // f<t...>(auto...)
        {
            constexpr auto f = []<typename...>(auto...){};

            static_assert(requires{ ns::apply(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<char>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<int, char>(f, std::tuple{'a'}); });
            
            static_assert(not noexcept(ns::apply(f, std::tuple{'a'})));
            static_assert(not noexcept(ns::apply<>(f, std::tuple{'a'})));
            static_assert(not noexcept(ns::apply<char>(f, std::tuple{'a'})));
            static_assert(not noexcept(ns::apply<int, char>(f, std::tuple{'a'})));
        }
        {
            constexpr auto f = []<typename...>(auto...) noexcept {};

            static_assert(requires{ ns::apply(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<char>(f, std::tuple{'a'}); });
            static_assert(requires{ ns::apply<int, char>(f, std::tuple{'a'}); });
            
            static_assert(noexcept(ns::apply(f, std::tuple{'a'})));
            static_assert(noexcept(ns::apply<>(f, std::tuple{'a'})));
            static_assert(noexcept(ns::apply<char>(f, std::tuple{'a'})));
            static_assert(noexcept(ns::apply<int, char>(f, std::tuple{'a'})));
        }
    }
}