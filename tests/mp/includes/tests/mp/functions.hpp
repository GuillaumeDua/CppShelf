#pragma once

#include <csl/mp.hpp>
#include <string>

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
namespace test::tuples::function::for_each {

    constexpr auto my_reduce(csl::mp::concepts::tuple_like auto && values){
        int reduced{};
        csl::mp::for_each(values, [&reduced](const auto & value){ reduced += value; });
        return reduced;
    }

    constexpr auto expected = 6;
    static_assert(expected == my_reduce(csl::mp::tuple{1,2,3}));
    static_assert(expected == my_reduce(std::tuple{1,2,3}));
    static_assert(expected == my_reduce(std::array{1,2,3}));
    static_assert(expected == my_reduce(std::pair{2,4}));
}
namespace test::tuples::function::for_each_enumerate {
    struct result_type {
        std::size_t index{};
        int value{};

        constexpr bool operator==(const result_type &) const noexcept = default;
    };
    constexpr auto my_indexed_reduce(csl::mp::concepts::tuple_like auto && values){
        result_type result{};
        csl::mp::for_each_enumerate(
            values,
             [&result](std::size_t i, const auto & value) {
                result.index += i;
                result.value += value;
            }
        );
        return result;
    }

    constexpr auto expected = result_type{ .index = 1, .value = 5 };
    static_assert(expected == my_indexed_reduce(csl::mp::tuple{2,3}));
    static_assert(expected == my_indexed_reduce(std::tuple{2,3}));
    static_assert(expected == my_indexed_reduce(std::array{2,3}));
    static_assert(expected == my_indexed_reduce(std::pair{2,3}));
}
// WIP -> std::invoke to expand API for F, elements... -> see https://godbolt.org/z/Ej3sz35GT
namespace test::tuples::function::apply::concepts {

    using throw_f_t = decltype([](int, int){});
    using nothrow_f_t = decltype([](int, int) noexcept {});

    // can_apply
    static_assert(csl::mp::concepts::can_apply<throw_f_t, csl::mp::tuple<int, int>>);
    static_assert(csl::mp::concepts::can_apply<throw_f_t, std::tuple<int, int>>);
    static_assert(csl::mp::concepts::can_apply<throw_f_t, std::array<int, 2>>);

    // can_nothrow_apply
    static_assert(not csl::mp::concepts::can_nothrow_apply<throw_f_t, csl::mp::tuple<int, int>>);
    static_assert(not csl::mp::concepts::can_nothrow_apply<throw_f_t, std::tuple<int, int>>);
    static_assert(not csl::mp::concepts::can_nothrow_apply<throw_f_t, std::array<int, 2>>);
    static_assert(csl::mp::concepts::can_nothrow_apply<nothrow_f_t, csl::mp::tuple<int, int>>);
    static_assert(csl::mp::concepts::can_nothrow_apply<nothrow_f_t, std::tuple<int, int>>);
    static_assert(csl::mp::concepts::can_nothrow_apply<nothrow_f_t, std::array<int, 2>>);
}
namespace test::tuples::function::apply {

    constexpr auto my_reduce = [](auto ... values){ return (0 + ... + values); };

    constexpr auto expected = 6;
    static_assert(expected == csl::mp::apply(my_reduce, csl::mp::tuple{1,2,3}));
    static_assert(expected == csl::mp::apply(my_reduce, std::tuple{1,2,3}));
    static_assert(expected == csl::mp::apply(my_reduce, std::array{1,2,3}));
    static_assert(expected == csl::mp::apply(my_reduce, std::pair{2,4}));
}
