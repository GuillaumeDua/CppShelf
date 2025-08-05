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

    static_assert(csl::mp::algorithm::concepts::can_apply<decltype(f_lvalue), tuple_type&>);
    static_assert(csl::mp::algorithm::concepts::can_apply<decltype(f_rvalue), tuple_type&&>);
    static_assert(csl::mp::algorithm::concepts::can_apply<decltype(f_const_lvalue), const tuple_type &>);
    static_assert(csl::mp::algorithm::concepts::can_apply<decltype(f_const_rvalue), const tuple_type &&>);

    constexpr void invoke(){
        auto value = tuple_type{};
        static_assert(requires { csl::mp::algorithm::apply(f_lvalue, value); });
        static_assert(requires { csl::mp::algorithm::apply(f_rvalue, std::move(value)); });
        static_assert(requires { csl::mp::algorithm::apply(f_const_lvalue, std::as_const(value)); });
        static_assert(requires { csl::mp::algorithm::apply(f_const_rvalue, static_cast<const tuple_type &&>(value)); });
    }
}
// TODO: static_assert result type
namespace test::tuples::algorithm::fold {

    static_assert(csl::mp::algorithm::fold_left(std::plus<void>{}, std::tuple{},     0) == 0);
    static_assert(csl::mp::algorithm::fold_left(std::plus<void>{}, csl::mp::tuple{}, 0) == 0);

    static_assert(csl::mp::algorithm::fold_left(
        std::plus<void>{},
        std::array{ 0, 1, 2, 3, 4, 5 },
        int{}
    ) == 15);
    static_assert(csl::mp::algorithm::fold_right(
        std::plus<void>{},
        std::array{ 0, 1, 2, 3, 4, 5 },
        int{}
    ) == 15);

    static_assert(std::invocable<std::plus<void>, std::string, char>);
    static_assert(std::invocable<std::plus<void>, std::string, std::string>);
    static_assert(std::invocable<std::plus<void>, std::string, const char *>);
    #if __cpp_lib_string_view >= 202403
    static_assert(std::invocable<std::plus<void>, std::string, std::string_view>); // requires C++26 - P2591
    #endif
    
    constexpr auto value = std::tuple{
            'a',
            std::string{ "bcdefgh" },
            "ijklmn",
        #if __cpp_lib_string_view >= 202403
            std::string_view{ "o" }
        #else
            "o"
        #endif
    };

    static_assert(csl::mp::algorithm::fold_left(
        std::plus<void>{},
        value,
        std::string{}
    ) == "abcdefghijklmno");
    static_assert(csl::mp::algorithm::fold_right(
        std::plus<void>{},
        value,
        std::string{}
    ) == "onmlkjihgfedcba");
}
