#pragma once
#include <csl/mp.hpp>
#include <string>
#include <cstdint>

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

namespace test::tuples::algorithm::contains {
    static_assert(not csl::mp::contains_v<csl::mp::tuple<>, int>);
    static_assert(csl::mp::contains_v<csl::mp::tuple<int>, int>);
    static_assert(csl::mp::contains_v<csl::mp::tuple<int, char>, int>);
    static_assert(csl::mp::contains_v<csl::mp::tuple<char, int>, int>);

    // std tuple-likes
    static_assert(csl::mp::contains_v<std::tuple<char, int>, int>);
    static_assert(csl::mp::contains_v<std::pair<char, int>, int>);
    static_assert(csl::mp::contains_v<std::array<int, 2>, int>);

    // with duplicates
    static_assert(csl::mp::contains_v<csl::mp::tuple<int, char, int>, int>);
    static_assert(csl::mp::contains_v<    std::tuple<int, char, int>, int>);
}
namespace test::tuples::algorithm::filter {
    static_assert(std::same_as<
        csl::mp::filter_t<
            csl::mp::tuple<int, double, float, char>,
            std::is_integral
        >,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::same_as<
        csl::mp::filter_t<
            std::tuple<int, double, float, char>,
            std::is_integral
        >,
        std::tuple<int, char>
    >);
}