#pragma once

#include "../../workflow/workflow.hpp"

namespace test::route_ {
    auto node_1 = [](){ return 41; };
    auto node_2 = [](int i){ return i + 1; };
    auto node_3 = [](){};

    # pragma region fwd (node_1 >[int]> node_2)
    // std::invocable
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1 })
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2 })
    >);

    // invoke_result_t
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1 })>,
        int
    >);
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1, node_2 })>,
        int
    >);
    #pragma endregion

    # pragma region fwd, discard (node_1 >[int]> node_2 >> node_3)
    // requires discard feature
    //
    // static_assert(std::invocable<
    //     decltype(csl::wf::route{ node_1, node_2, node_3 }),
    //     int
    // >);
    // static_assert(std::invocable<
    //     invoke_result_t<decltype(csl::wf::route{ node_1, node_2, node_3 })>,
    //     int
    // >);
    #pragma endregion
}