#pragma once
#include <csl/mp.hpp>
#include <string>
#include <cstdint>

namespace test::tuples::algorithm::apply_ {
    using tuple_type = csl::mp::tuple<std::size_t, std::string>;

    constexpr auto f_lvalue = [](std::size_t i, std::string &  str){ return i + str.length(); };
    constexpr auto f_rvalue = [](std::size_t i, std::string && str){ return i + str.length(); };
    constexpr auto f_const_lvalue = [](std::size_t i, const std::string &  str){ return i + str.length(); };
    constexpr auto f_const_rvalue = [](std::size_t i, const std::string && str){ return i + str.length(); };

    static_assert(csl::mp::concepts::can_apply<decltype(f_lvalue), tuple_type&>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_rvalue), tuple_type&&>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_const_lvalue), const tuple_type &>);
    static_assert(csl::mp::concepts::can_apply<decltype(f_const_rvalue), const tuple_type &&>);

    constexpr void invoke(){
        auto value = tuple_type{};
        static_assert(requires { csl::mp::apply(f_lvalue, value); });
        static_assert(requires { csl::mp::apply(f_rvalue, std::move(value)); });
        static_assert(requires { csl::mp::apply(f_const_lvalue, std::as_const(value)); });
        static_assert(requires { csl::mp::apply(f_const_rvalue, static_cast<const tuple_type &&>(value)); });
    }
}
// TODO: static_assert result type
namespace test::tuples::algorithm::fold::homogeneous {

    static_assert(csl::mp::fold_left(std::tuple{},     std::plus<void>{}, 0) == 0);
    static_assert(csl::mp::fold_left(csl::mp::tuple{}, std::plus<void>{}, 0) == 0);

    constexpr auto expected_sum = 15;
    static_assert(csl::mp::fold_left(
        std::array{ 0, 1, 2, 3, 4, 5 },
        std::plus<void>{},
        int{}
    ) == expected_sum);
    static_assert(csl::mp::fold_right(
        std::array{ 0, 1, 2, 3, 4, 5 },
        std::plus<void>{},
        int{}
    ) == expected_sum);
}
namespace test::tuples::algorithm::fold::heterogeneous {

    static_assert(std::invocable<std::plus<void>, std::string, char>);
    static_assert(std::invocable<std::plus<void>, std::string, std::string>);
    static_assert(std::invocable<std::plus<void>, std::string, const char *>);
    #if __cpp_lib_string_view >= 202403
    static_assert(std::invocable<std::plus<void>, std::string, std::string_view>); // requires C++26 - P2591
    #endif
    
    constexpr auto value = std::tuple{
            'a',
            std::string{ "bc" },
            "de",
        #if __cpp_lib_string_view >= 202403
            std::string_view{ "fg" }
        #else
            "fg"
        #endif
    };

    constexpr auto fold_left_result = csl::mp::fold_left(
        value,
        std::plus<void>{},
        std::string{}
    );
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(fold_left_result)>,
        csl::mp::fold_left_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::same_as<decltype(fold_left_result), const std::string>);
    static_assert(fold_left_result == "abcdefg");

    constexpr auto fold_right_result = csl::mp::fold_right(
        value,
        std::plus<void>{},
        std::string{}
    );
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(fold_right_result)>,
        csl::mp::fold_right_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::same_as<decltype(fold_right_result), const std::string>);
    static_assert(fold_right_result == "fgdebca");
}
