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
        using namespace workflow::functional;

        // f()
        {
            constexpr auto f = [](){};

            static_assert(requires{ apply(f, std::tuple{}); });
            static_assert(requires{ apply<>(f, std::tuple{}); });

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
}