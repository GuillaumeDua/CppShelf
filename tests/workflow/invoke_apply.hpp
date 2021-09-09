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

    struct A{};
    struct B{};

    consteval void applyable_before() {
        using namespace workflow::functional::mp;

        // f()
        {
            auto f = [](){};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, std::tuple<>>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>>);
            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<>>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>>);
        }
        {
            auto f = []() noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, std::tuple<>>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>>);
            static_assert(is_nothrow_applyable_before_v<F, std::tuple<>>);
            static_assert(is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>>);
        }
        // f(...)
        {
            auto f = [](B, A){};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, std::tuple<B, A>>);

            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B, A>>);

            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<>, B, A>);
            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<B>, A>);
            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<B, A>>);

            static_assert(not is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>, B, A>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<>, std::tuple<B>, A>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<>, std::tuple<B, A>>);
        }
        {
            auto f = [](B, A) noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, std::tuple<B, A>>);

            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<B, A>>);

            static_assert(is_nothrow_applyable_before_v<F, std::tuple<>, B, A>);
            static_assert(is_nothrow_applyable_before_v<F, std::tuple<B>, A>);
            static_assert(is_nothrow_applyable_before_v<F, std::tuple<B, A>>);

            static_assert(is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>, B, A>);
            static_assert(is_nothrow_applyable_before_v<F, ttps<>, std::tuple<B>, A>);
            static_assert(is_nothrow_applyable_before_v<F, ttps<>, std::tuple<B, A>>);
        }
        // f<T>(T)
        {
            auto f = []<typename T>(T){};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, std::tuple<>, B>);
            static_assert(is_applyable_before_v<F, std::tuple<B>>);

            static_assert(is_applyable_before_v<F, ttps<>, std::tuple<>, B>);
            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B>>);

            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<>, B>);
            static_assert(not is_nothrow_applyable_before_v<F, std::tuple<B>>);

            static_assert(not is_nothrow_applyable_before_v<F, ttps<>, std::tuple<>, B>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B>>);
        }
        {
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
        }
        // f<T>(...)
        {
            auto f = []<typename>(B, A){};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B, A>>);

            static_assert(not is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<>, B, A>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B>, A>);
            static_assert(not is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B, A>>);
        }
        {
            auto f = []<typename>(B, A) noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<>, B, A>);
            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B>, A>);
            static_assert(is_applyable_before_v<F, ttps<B>, std::tuple<B, A>>);

            static_assert(is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<>, B, A>);
            static_assert(is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B>, A>);
            static_assert(is_nothrow_applyable_before_v<F, ttps<B>, std::tuple<B, A>>);
        }
    }
    consteval void applyable_after() {
        using namespace workflow::functional::mp;

        // f()
        {
            auto f = [](){};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, std::tuple<>>);
            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>>);
            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<>>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>>);
        }
        {
            auto f = []() noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, std::tuple<>>);
            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>>);
            static_assert(is_nothrow_applyable_after_v<F, std::tuple<>>);
            static_assert(is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>>);
        }
        // f(...)
        {
            auto f = [](A, B){};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, std::tuple<>, A, B>);
            static_assert(is_applyable_after_v<F, std::tuple<B>, A>);
            static_assert(is_applyable_after_v<F, std::tuple<A, B>>);

            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>, A, B>);
            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<B>, A>);
            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<A, B>>);

            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<>, A, B>);
            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<A>, B>);
            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<A, B>>);

            static_assert(not is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>, A, B>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<>, std::tuple<A>, B>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<>, std::tuple<A, B>>);
        }
        {
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
        }
        // f<T>(T)
        {
            auto f = []<typename T>(T){};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, std::tuple<>, B>);
            static_assert(is_applyable_after_v<F, std::tuple<B>>);

            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>, B>);
            static_assert(is_applyable_after_v<F, ttps<B>, std::tuple<B>>);

            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<>, B>);
            static_assert(not is_nothrow_applyable_after_v<F, std::tuple<B>>);

            static_assert(not is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>, B>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<B>, std::tuple<B>>);
        }
        {
            auto f = []<typename T>(T) noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, std::tuple<>, B>);
            static_assert(is_applyable_after_v<F, std::tuple<B>>);

            static_assert(is_applyable_after_v<F, ttps<>, std::tuple<>, B>);
            static_assert(is_applyable_after_v<F, ttps<B>, std::tuple<B>>);

            static_assert(is_nothrow_applyable_after_v<F, std::tuple<>, B>);
            static_assert(is_nothrow_applyable_after_v<F, std::tuple<B>>);

            static_assert(is_nothrow_applyable_after_v<F, ttps<>, std::tuple<>, B>);
            static_assert(is_nothrow_applyable_after_v<F, ttps<B>, std::tuple<B>>);
        }
        // f<T>(...)
        {
            auto f = []<typename>(A, B){};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<>, A, B>);
            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<B>, A>);
            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<A, B>>);

            static_assert(not is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<>, A, B>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<B>, A>);
            static_assert(not is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<A, B>>);
        }
        {
            auto f = []<typename>(A, B) noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<>, A, B>);
            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<B>, A>);
            static_assert(is_applyable_after_v<F, ttps<int>, std::tuple<A, B>>);

            static_assert(is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<>, A, B>);
            static_assert(is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<B>, A>);
            static_assert(is_nothrow_applyable_after_v<F, ttps<int>, std::tuple<A, B>>);
        }
    }
}