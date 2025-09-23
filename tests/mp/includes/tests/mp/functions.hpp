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

    // BUG? std::get ?
    static_assert(std::addressof(i) == std::addressof(get<0>(tied)));
    static_assert(std::addressof(c) == std::addressof(get<1>(tied)));
}
namespace test::function::factory::make_tuple {

    [[maybe_unused]] static void impl(){

        int i{};
        const char c{};
        auto value = csl::mp::make_tuple(i, c, std::move(i)); // NOLINT(*-move-const-arg)

        static_assert(std::same_as<
            decltype(value),
            csl::mp::tuple<int, char, int>
        >);
    }
}
namespace test::function::factory::forward_as_tuple {

    [[maybe_unused]] static void impl(){

        int i{};
        const char c{};
        auto value = csl::mp::forward_as_tuple(i, c, std::move(i)); // NOLINT(*-move-const-arg)

        static_assert(std::same_as<
            decltype(value),
            csl::mp::tuple<int &, const char&, int&&>
        >);
    }
}
namespace test::function::factory::cat {
}
// for_each
// for_each_enumerate
// apply