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
namespace test::function::factory::cat_result {
    using empty = csl::mp::tuple<>;

    static_assert(std::same_as<empty, csl::mp::cat_result_t<>>);
    static_assert(std::same_as<empty, csl::mp::cat_result_t<empty>>);
    static_assert(std::same_as<empty, csl::mp::cat_result_t<empty, empty>>);

    static_assert(std::same_as<
        csl::mp::tuple<int, char, double>,
        csl::mp::cat_result_t<
            csl::mp::tuple<int>,
            csl::mp::tuple<char, double>
        >
    >);
    static_assert(std::same_as<
        csl::mp::tuple<int, char, double, bool>,
        csl::mp::cat_result_t<
            csl::mp::tuple<int>,
            std::pair<char, double>,
            std::tuple<bool>
        >
    >);
}
// for_each
// for_each_enumerate
// apply