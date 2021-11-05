#pragma once

#include "../../workflow/workflow.hpp"

namespace test::route_ {
    auto node_1 = [](){ return 41; };
    auto node_2 = [](int i){ return i + 1; };
    auto node_3 = [](){};

#pragma region fwd (node_1 >[int]> node_2)
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
#pragma region fwd (node_2 >[int]> node_2 >[int]> node_2) 
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2 }),
        int
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2, node_2 }),
        int
    >);
#pragma endregion

#pragma region discard (node_1 >[int]> node_3)
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_3 })
    >);
#pragma endregion
#pragma region fwd, discard (node_1 >[int]> node_2 >> node_3)
   
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2, node_3 })
    >);

#pragma endregion
}