#pragma once

#include <csl/workflow.hpp>

namespace test::wf::repeat_ {

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
    consteval void make_repetition_with_return() {
        constexpr auto result = csl::wf::make_repetition<3>([i = 0]() constexpr mutable { return ++i; })();
        static_assert(result == std::array{1,2,3});
    }
}