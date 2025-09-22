#pragma once

#include <csl/mp.hpp>

namespace test::function::factory::tie {

    constexpr int i{};
    constexpr char c{};
    constexpr auto tied = csl::mp::tie(i, c);

    static_assert(std::same_as<
        decltype(tied),
        const csl::mp::tuple<const int&, const char&>
    >);
}
namespace test::function::factory::make_tuple {
}
namespace test::function::factory::forward_as_tuple {
}
namespace test::function::factory::cat {
}