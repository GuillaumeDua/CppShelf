#pragma once

#include <csl/wf.hpp>

namespace test::wf::operators::star {

    consteval void with_no_return() {
        constexpr auto result = [](){
            std::array<int, 3> value;

            {
                using namespace csl::wf::operators;
                auto func_3_times =
                        [&, index = 0]() mutable { value[index] = index; index++; }
                    *   std::integral_constant<int, 3>{}
                ;
                func_3_times();
            }
            return value;
        }();
        static_assert(result == std::array{0,1,2});
    }
    consteval void invoke_n_times_with_return_value() {
        using namespace csl::wf::operators;

        constexpr auto result = (
                [i = 0]() constexpr mutable { return ++i; }
            *   std::integral_constant<std::size_t, 3>{}
        )();
        static_assert(result == std::array{1,2,3});
    }
    consteval void invoke_n_times_ttps() {
        constexpr auto func = []<typename T1, typename T2>(){
            return std::same_as<T1, T2>;
        };
        constexpr auto three_times = std::integral_constant<std::size_t, 3>{};

        using namespace csl::wf::operators;
        constexpr auto func_3_times = func * three_times;

        static_assert(std::array{true, true, true} == func_3_times.template operator()<int, int>());
        static_assert(std::array{false, false, false} == func_3_times.template operator()<char, int>());
    }

    consteval void times_operator() {
        using namespace csl::wf::operators;
        using namespace csl::wf::literals;

        constexpr auto result = (
                [i = 0]() constexpr mutable { return ++i; }
            *   3_times
        )();
        static_assert(result == std::array{1,2,3});
    }
    // repeater flattening
    consteval void repeater_flattening() {
        using namespace csl::wf::operators;
        using namespace csl::wf::literals;

        constexpr auto func = [](){};
        constexpr auto value = func * 3_times * 1_times * 3_times * 1_times;

        static_assert(decltype(value)::times == 9);
        static_assert(std::same_as<decltype(value)::underlying_type, std::remove_cvref_t<decltype(func)>>);

        static_assert(std::same_as<
            const csl::wf::repeater<std::size_t{9}, std::remove_cvref_t<decltype(func)>>,
            decltype(value)
        >);
    }
}
