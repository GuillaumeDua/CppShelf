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
// for_each
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

}
namespace test::tuples::function::apply {
    using tuple_type = csl::mp::tuple<std::size_t, std::string>;

    constexpr auto f_lvalue = [](std::size_t i, std::string &  str){ return i + str.length(); };
    constexpr auto f_rvalue = [](std::size_t i, std::string && str){ return i + str.length(); }; // NOLINT(*-not-moved)
    constexpr auto f_const_lvalue = [](std::size_t i, const std::string &  str){ return i + str.length(); };
    constexpr auto f_const_rvalue = [](std::size_t i, const std::string && str){ return i + str.length(); };

    static_assert(csl::mp::concepts::can_apply<decltype(f_lvalue), tuple_type&>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_rvalue), tuple_type&&>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_const_lvalue), const tuple_type &>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_const_rvalue), const tuple_type &&>);

    constexpr void impl(){
        auto value = tuple_type{};
        static_assert(requires { csl::mp::apply(f_lvalue, value); });
        static_assert(requires { csl::mp::apply(f_rvalue, std::move(value)); });
        static_assert(requires { csl::mp::apply(f_const_lvalue, std::as_const(value)); });
        static_assert(requires { csl::mp::apply(f_const_rvalue, static_cast<const tuple_type &&>(value)); });
    }
}