#pragma once

#include <csl/wf.hpp>

namespace test {

    consteval void invocable() {
        using namespace csl::wf::mp;

        // f()
        {
            constexpr auto f = [](){};
            using F = decltype(f);

            static_assert(is_invocable_v<F>);
            static_assert(is_invocable_v<F, ttps<>>);
            static_assert(not is_nothrow_invocable_v<F>);
            static_assert(not is_nothrow_invocable_v<F, ttps<>>);
            static_assert(invocable_with<F, ttps<>, args<>>);
            static_assert(not nothrow_invocable_with<F, ttps<>, args<>>);
        }
        {
            constexpr auto f = []() noexcept {};
            using F = decltype(f);

            static_assert(is_invocable_v<F>);
            static_assert(is_invocable_v<F, ttps<>>);
            static_assert(is_nothrow_invocable_v<F>);
            static_assert(is_nothrow_invocable_v<F, ttps<>>);
            static_assert(invocable_with<F, ttps<>, args<>>);
            static_assert(nothrow_invocable_with<F, ttps<>, args<>>);
        }
        // f(...)
        {
            constexpr auto f_with_args = [](int, bool){};
            using F = decltype(f_with_args);

            static_assert(is_invocable_v<F, int, bool>);
            static_assert(is_invocable_v<F, ttps<>, int, bool>);
            static_assert(not is_nothrow_invocable_v<F, int, bool>);
            static_assert(not is_nothrow_invocable_v<F, ttps<>, int, bool>);
        }
        {
            constexpr auto f_with_args = [](int, bool) noexcept {};
            using F = decltype(f_with_args);

            static_assert(is_invocable_v<F, int, bool>);
            static_assert(is_invocable_v<F, ttps<>, int, bool>);
            static_assert(is_nothrow_invocable_v<F, int, bool>);
            static_assert(is_nothrow_invocable_v<F, ttps<>, int, bool>);
        }
        // f<...>()
        {
            constexpr auto f_with_ttps = []<typename, typename>(){};
            using F = decltype(f_with_ttps);

            static_assert(is_invocable_v<F, ttps<int, int>>);
        }
        // f<T>(T)
        {
            constexpr auto f_with_deductible_ttps = []<typename T>(T){};
            using F = decltype(f_with_deductible_ttps);

            static_assert(is_invocable_v<F, int>);
            static_assert(is_invocable_v<F, ttps<int>, int>);
        }
        // f<t...>(auto...)
        {
            constexpr auto f_with_ttps = []<typename...>(auto...){};
            using F = decltype(f_with_ttps);

            static_assert(is_invocable_v<F, ttps<>>);
            static_assert(is_invocable_v<F, ttps<int>>);
            static_assert(is_invocable_v<F, ttps<int, int>>);
            static_assert(is_invocable_v<F, ttps<int, int>, bool>);
            static_assert(is_invocable_v<F, ttps<int, int>, bool, char>);

            static_assert(invocable_with<F, ttps<>, args<>>);
            static_assert(invocable_with<F, ttps<int>, args<>>);
            static_assert(invocable_with<F, ttps<>, args<int>>);
        }
    }
    consteval void applyable() {
        using namespace csl::wf::mp;

        // f()
        {
            constexpr auto f = [](){};
            using F = decltype(f);

            static_assert(is_applyable_v<F, std::tuple<>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<>>);
            static_assert(not is_nothrow_applyable_v<F, std::tuple<>>);
            static_assert(not is_nothrow_applyable_v<F, ttps<>, std::tuple<>>);
        }
        {
            constexpr auto f = []() noexcept {};
            using F = decltype(f);

            static_assert(is_applyable_v<F, std::tuple<>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<>>);
            static_assert(is_nothrow_applyable_v<F, std::tuple<>>);
            static_assert(is_nothrow_applyable_v<F, ttps<>, std::tuple<>>);
        }
        // f(...)
        {
            constexpr auto f_with_args = [](int, bool){};
            using F = decltype(f_with_args);

            static_assert(is_applyable_v<F, std::tuple<int, bool>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<int, bool>>);
            static_assert(not is_nothrow_applyable_v<F, std::tuple<int, bool>>);
            static_assert(not is_nothrow_applyable_v<F, ttps<>, std::tuple<int, bool>>);
        }
        {
            constexpr auto f_with_args = [](int, bool) noexcept {};
            using F = decltype(f_with_args);

            static_assert(is_applyable_v<F, std::tuple<int, bool>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<int, bool>>);
            static_assert(is_nothrow_applyable_v<F, std::tuple<int, bool>>);
            static_assert(is_nothrow_applyable_v<F, ttps<>, std::tuple<int, bool>>);
        }
        // f<...>()
        {
            constexpr auto f_with_ttps = []<typename, typename>(){};
            using F = decltype(f_with_ttps);

            static_assert(is_applyable_v<F, ttps<int, int>, std::tuple<>>);
        }
        // f<T>(T)
        {
            constexpr auto f_with_deductible_ttps = []<typename T>(T){};
            using F = decltype(f_with_deductible_ttps);

            static_assert(is_applyable_v<F, std::tuple<int>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<int>>);
            static_assert(is_applyable_v<F, ttps<int>, std::tuple<int>>);
        }
        // f<t...>(auto...)
        {
            constexpr auto f_with_ttps = []<typename...>(auto...){};
            using F = decltype(f_with_ttps);

            static_assert(is_applyable_v<F, ttps<>, std::tuple<>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<int>>);
            static_assert(is_applyable_v<F, ttps<>, std::tuple<int, char>>);

            static_assert(is_applyable_v<F, ttps<int>, std::tuple<>>);
            static_assert(is_applyable_v<F, ttps<int, char>, std::tuple<>>);
        }
    }
    consteval void invocable_result() {
        using namespace csl::wf::mp;

        {
            const auto f = []<typename T = std::true_type>(auto arg = std::true_type{}) constexpr {
                using arg_type = std::remove_cvref_t<decltype(arg)>;
                if constexpr (T::value and arg_type::value)
                    return std::true_type{};
                else return std::false_type{};
            };
            using F = decltype(f);

            static_assert(std::same_as<
                std::true_type,
                invoke_result_t<F, std::true_type>
            >);
            static_assert(std::same_as<
                std::false_type,
                invoke_result_t<F, ttps<std::true_type>, std::false_type>
            >);
            static_assert(std::same_as<
                std::true_type,
                invoke_result_t<F, ttps<std::true_type>, std::true_type>
            >);
        }
        {

        }
    }

    consteval void ttps_cvref_qualifiers() {
        auto func = []<typename ...>(){};
        using func_type = decltype(func);

        using namespace csl::wf::mp;
        static_assert(is_invocable_v<func_type>);
        static_assert(is_invocable_v<func_type, ttps<>>);
        static_assert(is_invocable_v<func_type, ttps<int>>);
        static_assert(is_invocable_v<func_type, ttps<>&>);
        static_assert(is_invocable_v<func_type, ttps<>&&>);
        static_assert(is_invocable_v<func_type, const ttps<>&>);
        static_assert(is_invocable_v<func_type, const ttps<>&&>);

        static_assert(is_applyable_v<func_type, std::tuple<>>);
        static_assert(is_applyable_v<func_type, std::tuple<ttps<int>>>);
        static_assert(is_applyable_v<func_type, std::tuple<ttps<>>>);
        static_assert(is_applyable_v<func_type, std::tuple<ttps<>&>>);
        static_assert(is_applyable_v<func_type, std::tuple<ttps<>&&>>);
        static_assert(is_applyable_v<func_type, std::tuple<const ttps<>&>>);
        static_assert(is_applyable_v<func_type, std::tuple<const ttps<>&&>>);
    }
}

namespace test::details_mp {
    consteval void invocable_with_ttps_args() {
        using namespace csl::wf::mp;

        const auto f_witht_ttps = []<typename T>(bool, int){};
        static_assert(invocable_with<decltype(f_witht_ttps), ttps<int>, args<bool, int>>);

        const auto f_void = []<typename T>(){};
        static_assert(    invocable_with<decltype(f_void), ttps<int>, args<>>);
        static_assert(not invocable_with<decltype(f_void), ttps<int>, args<void>>);
    }
}
