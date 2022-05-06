#pragma once

#include <csl/wf.hpp>
#include <tuple>

namespace test::_mp::_tuple_view {

    static_assert(std::same_as<
        std::tuple<int&&, const char &&>,
        decltype(csl::wf::mp::make_tuple_view(std::tuple<int, const char>{}))
    >);

    static_assert(std::same_as<
        std::tuple<int &, const char &>,
        decltype(csl::wf::mp::make_tuple_view(std::tuple<int&, const char&>{
            std::declval<int&>(), std::declval<char&>()
        }))
    >);
    static_assert(std::same_as<
        std::tuple<>,
        decltype(csl::wf::mp::make_tuple_view(std::tuple<>{}))
    >);
}