#pragma once

#include <csl/wf.hpp>

namespace test::wf::repeat_ {

    // invoke_n_times
    consteval void invoke_n_times() {
        constexpr auto result = [](){
            std::array<int, 3> value;
            csl::wf::invoke_n_times<3>(
                [&, index = 0]() mutable { value[index] = index; index++; }
            );
            return value;
        }();
        static_assert(result == std::array{0,1,2});
    }
    consteval void invoke_n_times_with_return_value() {
        constexpr auto result = csl::wf::invoke_n_times<3>([i = 0]() constexpr mutable { return ++i; });
        static_assert(result == std::array{1,2,3});
    }

    // repeater
    

    // make_repetition
    consteval void make_repetition() {
        constexpr auto result = [](){
            std::array<int, 3> value;
            csl::wf::make_repetition<3>(
                [&, index = 0]() mutable { value[index] = index; index++; }
            )();
            return value;
        }();
        static_assert(result == std::array{0,1,2});
    }
    consteval void make_repetition_with_return_value() {
        constexpr auto result = csl::wf::make_repetition<3>([i = 0]() constexpr mutable { return ++i; })();
        static_assert(result == std::array{1,2,3});
    }
}