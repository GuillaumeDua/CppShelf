#pragma once 

#include <csl/wf.hpp>

namespace test::wf::literals::times {

    consteval void type_correctness() {
        using namespace csl::wf::literals;

        // 18-digits10-long
        static_assert(std::is_same_v<
            decltype(123123123123123123_times),
            std::integral_constant<std::size_t, 123123123123123123_times>
        >);
        // 19-digits10-long
        // using compilation_error = decltype(1231231231231231231_times); // compile-time error : out of range
    }
    consteval void invoke_n_times() {

        constexpr std::size_t counter = []() constexpr {
            auto value = 0;

            using namespace csl::wf::literals;
            using namespace csl::wf::operators;
            const auto count_n_times = [&value]() constexpr { ++value; } * 42_times;
            count_n_times();
            return value;
        }();
        static_assert(counter == 42);
    }
}