#pragma once
#include <csl/mp.hpp>
#include <string>
#include <cstdint>

namespace test::tuples::algorithm {
    struct A{};
    struct B{};
    struct C{};
    struct D{};
    struct E{};
    struct F{};
    struct G{};
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

namespace test::tuples::algorithm::is_uniqued {
    using valid_tuple = csl::mp::tuple<int, char>;
    static_assert(csl::mp::is_uniqued_v<valid_tuple>);

    using invalid_tuple = csl::mp::tuple<int, char, int>;
    static_assert(not csl::mp::is_uniqued_v<invalid_tuple>);
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

    using csl_tuple = csl::mp::tuple<int, double, float, char>;
    using std_tuple = csl::mp::unfold_t<csl_tuple, std::tuple>;

    static_assert(std::same_as<
        csl::mp::filter_t<csl_tuple, std::is_integral>,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::same_as<
        csl::mp::filter_t<std_tuple, std::is_integral >,
        std::tuple<int, char>
    >);
    static_assert(std::same_as<
        csl::mp::filter_t<csl_tuple, std::is_floating_point>,
        csl::mp::tuple<double, float>
    >);
}
// WIP: std::tuple support
namespace test::tuples::algorithm::set_union {
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_union_t<T0, T1>,
        csl::mp::tuple<int, char, double>
    >);
    static_assert(std::is_same_v<
        csl::mp::set_union_t<T0, csl::mp::unfold_t<T1, std::tuple>>,
        csl::mp::tuple<int, char, double>
    >);
}
namespace test::tuples::algorithm::set_intersection {
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_intersection_t<T0, T1>,
        csl::mp::tuple<int>
    >);
}
namespace test::tuples::algorithm::set_difference {
    
    static_assert(std::is_same_v<
        csl::mp::set_difference_t<
            csl::mp::tuple<int, char>,
            csl::mp::tuple<int, double>
        >,
        csl::mp::tuple<char>
    >);

    static_assert(std::is_same_v<
        csl::mp::set_difference_t<
            csl::mp::tuple<A,B,D,D,D,G>,
            csl::mp::tuple<B,D,F>
        >,
        csl::mp::tuple<A,D,D,G>
    >);
}
namespace test::tuples::algorithm::deduplicate {
    using csl_tuple_valid   = csl::mp::tuple<int, char, double>;
    using csl_tuple_invalid = csl::mp::tuple<int, char, int, char, double, int>;
    
    static_assert(std::is_same_v<
        csl_tuple_valid,
        csl::mp::deduplicate_t<csl_tuple_valid>
    >);
    static_assert(std::is_same_v<
        csl_tuple_valid,
        csl::mp::deduplicate_t<csl_tuple_invalid>
    >);

    using std_tuple_valid   = csl::mp::unfold_t<csl::mp::tuple<int, char, double>, std::tuple>;
    using std_tuple_invalid = csl::mp::unfold_t<csl::mp::tuple<int, char, int, char, double, int>, std::tuple>;

    static_assert(std::is_same_v<
        std_tuple_valid,
        csl::mp::deduplicate_t<std_tuple_valid>
    >);
    static_assert(std::is_same_v<
        std_tuple_valid,
        csl::mp::deduplicate_t<std_tuple_invalid>
    >);

}


// TODO(Guillaume) sort, is_sorted
