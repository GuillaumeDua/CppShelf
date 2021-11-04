#pragma once

#include "../../workflow/workflow.hpp"

namespace test::route_ {
    auto node_1 = [](){ return 41; };
    auto node_2 = [](int i){ return i + 1; };
    auto node_3 = [](){};

    // std::invocable
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1 }),
        int
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2 }),
        int
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2, node_3 }),
        int
    >);

    // invoke_result_t
    static_assert(std::same_as<
        invoke_result_t<decltype(csl::wf::route{ node_1 })>,
        int
    >);
    static_assert(std::invocable<
        invoke_result_t<decltype(csl::wf::route{ node_1, node_2 })>,
        int
    >);
    static_assert(std::invocable<
        invoke_result_t<decltype(csl::wf::route{ node_1, node_2, node_3 })>,
        int
    >);
}