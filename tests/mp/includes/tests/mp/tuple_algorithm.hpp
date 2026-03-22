#pragma once
#include <csl/mp.hpp>
#include <tuple>
#include <array>
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

namespace test::tuples::algorithm::count {
    using t = csl::mp::tuple<int, char, bool, int, double>;
    static_assert(0 == csl::mp::count_v<t, float>);
    static_assert(1 == csl::mp::count_v<t, char>);
    static_assert(2 == csl::mp::count_v<t, int>);

    // empty tuple
    static_assert(0 == csl::mp::count_v<csl::mp::tuple<>, int>);

    static_assert(1 == csl::mp::count_v<std::tuple<int, char>, char>);
}
namespace test::tuples::algorithm::count_if {
    using t = csl::mp::tuple<int, char, bool, double, float>;
    static_assert(3 == csl::mp::count_if_v<t, std::is_integral>);
    static_assert(2 == csl::mp::count_if_v<t, std::is_floating_point>);

    // empty tuple
    using is_int64_t = csl::mp::bind_front<std::is_same, std::int64_t>;
    static_assert(0 == csl::mp::count_if_v<t, is_int64_t::type>);
}
namespace test::tuples::algorithm::uniqued {

    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(csl::mp::is_uniqued_v<without_duplicates>);
    static_assert(not csl::mp::is_uniqued_v<with_duplicates>);

    static_assert(not csl::mp::concepts::uniqued<with_duplicates>);
    static_assert(csl::mp::concepts::uniqued<without_duplicates>);
    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<int>>);

    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<int>&>);
    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<int>&&>);
    static_assert(csl::mp::concepts::uniqued<const csl::mp::tuple<int>&>);

    static_assert(csl::mp::concepts::uniqued<std::array<int, 1>>);
    static_assert(not csl::mp::concepts::uniqued<std::array<int, 2>>);
}
namespace test::tuples::algorithm::unfold {
    
    static_assert(std::is_same_v<
        csl::mp::unfold_t<csl::mp::tuple<int, char>, csl::mp::tuple>,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::unfold_t<std::tuple<int, char>, csl::mp::tuple>,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::unfold_t<std::array<int, 2>, csl::mp::tuple>,
        csl::mp::tuple<int, int>
    >);
    static_assert(std::is_same_v<
        csl::mp::unfold_t<std::array<int, 2>, std::tuple>,
        std::tuple<int, int>
    >);
    static_assert(std::is_same_v<
        csl::mp::unfold_t<csl::mp::tuple<int, char>, std::tuple>,
        std::tuple<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::unfold_t<csl::mp::tuple<int, char>, std::pair>,
        std::pair<int, char>
    >);

    static_assert(std::is_same_v<
        csl::mp::unfold_t<csl::mp::tuple<int, char>, std::is_same>,
        std::is_same<int, char>
    >);
}
namespace test::tuples::algorithm::rebind {
    static_assert(std::is_same_v<
        csl::mp::rebind_t<std::tuple<int>, char>,
        std::tuple<char>
    >);
    static_assert(std::is_same_v<
        csl::mp::rebind_t<csl::mp::tuple<int>, char>,
        csl::mp::tuple<char>
    >);
    static_assert(std::is_same_v<
        csl::mp::rebind_t<std::pair<int, char>, bool, double>,
        std::pair<bool, double>
    >);
    static_assert(std::is_same_v<
        csl::mp::rebind_t<std::array<int, 4>, char,char,char,char>,
        std::array<char, 4>
    >);

    static_assert(std::is_same_v<
        csl::mp::rebind_t<std::is_same<int, char>, bool, double>,
        std::is_same<bool, double>
    >);
}
namespace test::tuples::algorithm::transform {

    static_assert(std::is_same_v<
        csl::mp::transform_t<csl::mp::tuple<int, char>, std::add_lvalue_reference_t>,
        csl::mp::tuple<int&, char&>
    >);
    static_assert(std::is_same_v<
        csl::mp::transform_t<std::tuple<int, char>, std::add_lvalue_reference_t>,
        std::tuple<int&, char&>
    >);
    static_assert(std::is_same_v<
        csl::mp::transform_t<std::pair<int, char>, std::add_lvalue_reference_t>,
        std::pair<int&, char&>
    >);
    static_assert(std::is_same_v<
        csl::mp::transform_t<std::array<int, 1>, std::add_lvalue_reference_t>,
        std::array<int&, 1>
    >);
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

    static_assert(std::is_same_v<
        csl::mp::filter_t<csl_tuple, std::is_integral>,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::filter_t<std_tuple, std::is_integral>,
        std::tuple<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::filter_t<csl_tuple, std::is_floating_point>,
        csl::mp::tuple<double, float>
    >);
    static_assert(std::is_same_v<
        csl::mp::filter_t<std::pair<int, char>, std::is_integral>,
        std::pair<int, char>
    >);
    static_assert(std::is_same_v<
        csl::mp::filter_t<std::array<int, 2>, std::is_integral>,
        std::array<int, 2>
    >);
}
namespace test::tuples::algorithm::replace {

    struct replacement{};

    static_assert(std::is_same_v<
        csl::mp::replace_t<csl::mp::tuple<int, double, int>, int, replacement>,
        csl::mp::tuple<replacement, double, replacement>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_t<std::tuple<int, double, int>, int, replacement>,
        std::tuple<replacement, double, replacement>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_t<std::array<int, 2>, int, replacement>,
        std::array<replacement, 2>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_t<std::pair<int, char>, int, replacement>,
        std::pair<replacement, char>
    >);
}
namespace test::tuples::algorithm::replace_if {
    
    struct replacement{};

    static_assert(std::is_same_v<
        csl::mp::replace_if_t<csl::mp::tuple<int, double, char>, std::is_integral, replacement>,
        csl::mp::tuple<replacement, double, replacement>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_if_t<std::tuple<int, double, char>, std::is_integral, replacement>,
        std::tuple<replacement, double, replacement>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_if_t<std::array<int, 2>, std::is_integral, replacement>,
        std::array<replacement, 2>
    >);
    static_assert(std::is_same_v<
        csl::mp::replace_if_t<std::pair<int, char>, std::is_integral, replacement>,
        std::pair<replacement, replacement>
    >);
}

// WIP: std::tuple support
// WIP: result type should be:
//  T0, T1 are same      => T0
//  T0, T1 are different => T0 or csl::mp::tuple ?
namespace test::tuples::algorithm::set_union {

    namespace csl_mp_tuple {
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
    namespace std_tuple {
        using T0 = std::tuple<int, char>;
        using T1 = std::tuple<int, double>;
        
        static_assert(std::is_same_v<
            csl::mp::set_union_t<T0, T1>,
            csl::mp::tuple<int, char, double>
        >);
        static_assert(std::is_same_v<
            csl::mp::set_union_t<T0, csl::mp::unfold_t<T1, std::tuple>>,
            csl::mp::tuple<int, char, double>
        >);
    }
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

namespace test::tuples::algorithm::fold::homogeneous {

    // empty: both directions always return init, regardless of operation
    static_assert(csl::mp::fold_left (csl::mp::tuple{}, std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::fold_left (std::tuple{},     std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::fold_right(csl::mp::tuple{}, std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::fold_right(std::tuple{},     std::minus<void>{}, 0) == 0);

    // minus:
    //  single:     fold_left  = f(init, x) = 0 - 5 = -5
    //              fold_right = f(x, init) = 5 - 0 =  5
    //  multiple:   fold_left ({1,2,3}, -, 0) = ((0-1)-2)-3 = -6
    //              fold_right({1,2,3}, -, 0) =  1-(2-(3-0)) = 2
    static_assert(csl::mp::fold_left (csl::mp::tuple{5},   std::minus<void>{}, 0)   == -5);
    static_assert(csl::mp::fold_left (std::tuple{5},       std::minus<void>{}, 0)   == -5);
    static_assert(csl::mp::fold_right(csl::mp::tuple{5},   std::minus<void>{}, 0)   ==  5);
    static_assert(csl::mp::fold_right(std::tuple{5},       std::minus<void>{}, 0)   ==  5);
    static_assert(csl::mp::fold_left (std::array{1, 2, 3}, std::minus<void>{}, int{}) == -6);
    static_assert(csl::mp::fold_right(std::array{1, 2, 3}, std::minus<void>{}, int{}) ==  2);

    // plus:
    static_assert(csl::mp::fold_left (std::array{0,1,2,3,4,5}, std::plus<void>{}, int{}) == 15); // NOLINT(*-magic-numbers)
    static_assert(csl::mp::fold_right(std::array{0,1,2,3,4,5}, std::plus<void>{}, int{}) == 15); // NOLINT(*-magic-numbers)

    // result_type
    static_assert(std::is_same_v<csl::mp::fold_left_result_t <std::array<int,2>, std::plus<void>,  int>, int>);
    static_assert(std::is_same_v<csl::mp::fold_right_result_t<std::array<int,2>, std::plus<void>,  int>, int>);
    static_assert(std::is_same_v<csl::mp::fold_left_result_t <std::array<int,2>, std::minus<void>, int>, int>);
    static_assert(std::is_same_v<csl::mp::fold_right_result_t<std::array<int,2>, std::minus<void>, int>, int>);
}
namespace test::tuples::algorithm::fold::heterogeneous {

    static_assert(std::invocable<std::plus<void>, std::string, char>);
    static_assert(std::invocable<std::plus<void>, std::string, std::string>);
    static_assert(std::invocable<std::plus<void>, std::string, const char *>);
#if __cpp_lib_string_view >= 202403L
    static_assert(std::invocable<std::plus<void>, std::string, std::string_view>); // C++26 P2591
#endif

    constexpr auto value = std::tuple{
        'a',
#if __cpp_lib_constexpr_string >= 201907L
        std::string{ "bc" },
#else
        "bc",
#endif
        "de",
#if __cpp_lib_string_view >= 202403L
        std::string_view{ "fg" }
#else
        "fg"
#endif
    };

    static_assert(std::is_same_v<
        decltype(csl::mp::fold_left(value, std::plus<void>{}, std::string{})),
        csl::mp::fold_left_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::is_same_v<
        csl::mp::fold_left_result_t<decltype(value), std::plus<void>, std::string>,
        std::string
    >);
    static_assert(csl::mp::fold_left(value, std::plus<void>{}, std::string{}) == "abcdefg");

    static_assert(std::is_same_v<
        decltype(csl::mp::fold_right(value, std::plus<void>{}, std::string{})),
        csl::mp::fold_right_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::is_same_v<
        csl::mp::fold_right_result_t<decltype(value), std::plus<void>, std::string>,
        std::string
    >);
    static_assert(csl::mp::fold_right(value, std::plus<void>{}, std::string{}) == "abcdefg");
}
namespace test::tuples::algorithm::fold::accumulation_order {

    // GCC < 15: complain about that a named constexpr variable "not usable in a constant expression" due to basic_string internals.
    #if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 15)
    #else
        // expression_accumulator(a, b) = "(a,b)" to check internal behavior:
        //   fold_left  = f(f(f(init, x), y), z) = "(((_,x),y),z)"
        //   fold_right = f(x, f(y, f(z, init))) = "(x,(y,(z,_)))"
        constexpr auto expression_accumulator = [](std::string_view a, std::string_view b) -> std::string {
            return "(" + std::string{a} + "," + std::string{b} + ")";
        };
        using namespace std::string_literals;
        constexpr auto value = std::make_tuple("x"s, "y"s, "z"s);

        static_assert(csl::mp::fold_left( value, expression_accumulator, "_"s) == "(((_,x),y),z)");
        static_assert(csl::mp::fold_right(value, expression_accumulator, "_"s) == "(x,(y,(z,_)))");
    #endif
}

// TODO(Guillaume) sort, is_sorted

namespace test::tuples::algorithm::functions::all_any_none_of {

    namespace csl_tuple {
        constexpr auto value = csl::mp::tuple{ 42, .42f, 'a' };
        static_assert(csl::mp::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::none_of(value, [](const auto & element){ return element < 0; }));

        namespace with_rvalue {
            static_assert(csl::mp::all_of(csl::mp::tuple{ 42, .42f, 'a' }, [](const auto & element){ return element > 0; }));
            static_assert(csl::mp::all_of(csl::mp::make_tuple( 42, .42f, 'a' ), [](const auto & element){ return element > 0; }));
            static_assert(csl::mp::all_of(csl::mp::forward_as_tuple( 42, .42f, 'a' ), [](const auto & element){ return element > 0; }));
        }
    }
    namespace std_tuple {
        constexpr auto value = std::tuple{ 42, .42f, 'a' };
        static_assert(csl::mp::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::none_of(value, [](const auto & element){ return element < 0; }));
    }
    namespace std_array {
        constexpr auto value = std::array{ 42.F, .42F };
        static_assert(csl::mp::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::none_of(value, [](const auto & element){ return element < 0; }));
    }
}
