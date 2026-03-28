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

    // csl::mp::tuple
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<int, float>, char, double>,  csl::mp::tuple<char, double>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<int, float>, char>,          csl::mp::tuple<char>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<int, float>>,                csl::mp::tuple<>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<>>,                          csl::mp::tuple<>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<>, int, float>,              csl::mp::tuple<int, float>>);
    // element types from `like` are fully replaced
    static_assert(std::is_same_v<
        csl::mp::rebind_t<
            csl::mp::tuple<std::int64_t, std::int64_t, std::int64_t>,
            int, float, double
        >,
        csl::mp::tuple<int, float, double>
    >);

    // std::tuple
    static_assert(std::is_same_v<csl::mp::rebind_t<std::tuple<int, float>, char, double>,  std::tuple<char, double>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::tuple<int, float>>,                std::tuple<>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::tuple<>>,                          std::tuple<>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::tuple<>, int, float>,              std::tuple<int, float>>);

    // std::pair - always exactly 2 types
    static_assert(std::is_same_v<csl::mp::rebind_t<std::pair<int, float>, char, double>,               std::pair<char, double>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::pair<std::int64_t, std::int64_t>, int, float>, std::pair<int, float>>);
    // wrong arity is ill-formed:
    //   rebind_t<std::pair<int,float>, char>                       // 1 element  -> ill-formed
    //   rebind_t<std::pair<int,float>, char, double, std::int64_t> // 3 elements -> ill-formed

    // std::array
    //   Requires: sizeof...(Us) == N AND all Us are the same type.
    //   Both constraints are structural: violating either is ill-formed.
    static_assert(std::is_same_v<csl::mp::rebind_t<std::array<int, 3>, float, float, float>,   std::array<float, 3>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::array<int, 1>, double>,                std::array<double, 1>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<std::array<int, 2>, char, char>,            std::array<char, 2>>);
    // N mismatch is ill-formed:
    //   rebind_t<std::array<int,3>, float, float>                  // 2 != 3 -> ill-formed
    //   rebind_t<std::array<int,3>, float, float, float, float>    // 4 != 3 -> ill-formed
    // Heterogeneous Us is ill-formed:
    //   rebind_t<std::array<int,3>, float, int, float>             // not all same -> ill-formed

    // User-defined TTP (no specialisation needed: primary handles it)
    template <typename... Ts> struct pack {};
    static_assert(std::is_same_v<csl::mp::rebind_t<pack<int, float>, char, double>,    pack<char, double>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<pack<int, float>>,                  pack<>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<pack<>, int>,                       pack<int>>);

    // Us can carry cvref qualifiers regardless of what T holds
    static_assert(std::is_same_v<csl::mp::rebind_t<std::tuple<std::int64_t, std::int64_t>, int&, float&&>, std::tuple<int&, float&&>>);
    static_assert(std::is_same_v<csl::mp::rebind_t<csl::mp::tuple<double>,  const int>,                    csl::mp::tuple<const int>>);
}
namespace test::tuples::algorithm::rebind_elements {

    // base use-case
    static_assert(std::is_same_v<
        csl::mp::rebind_elements_t<
            std::tuple<std::int64_t,std::int64_t,std::int64_t>,
            csl::mp::tuple<int,float,double>
        >,
        std::tuple<int,float,double>
    >);

    // csl::mp::tuple
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<int,float>, csl::mp::tuple<char,double>>,  csl::mp::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<int,float>, std::tuple<char,double>>,      csl::mp::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<int,float>, std::pair<char,double>>,       csl::mp::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<int,float>, std::array<char,2>>,           csl::mp::tuple<char,char>>);

    // std::tuple
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::tuple<int,float>, csl::mp::tuple<char,double>>,      std::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::tuple<int,float>, std::tuple<char,double>>,          std::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::tuple<int,float>, std::pair<char,double>>,           std::tuple<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::tuple<int,float>, std::array<char,2>>,               std::tuple<char,char>>);

    // std::pair
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::pair<int,float>, csl::mp::tuple<char,double>>,       std::pair<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::pair<int,float>, std::tuple<char,double>>,           std::pair<char,double>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::pair<int,float>, std::array<char,2>>,                std::pair<char,char>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::pair<int,float>, std::pair<char,double>>,            std::pair<char,double>>);

    // std::array
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::array<int,2>, csl::mp::tuple<float,float>>,          std::array<float,2>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::array<int,2>, std::tuple<float,float>>,              std::array<float,2>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::array<int,2>, std::array<float,2>>,                  std::array<float,2>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::array<int,2>, std::pair<float,float>>,               std::array<float,2>>);
    // heterogeneous elements_source with std::array shape is ill-formed:
    //   rebind_elements_t<std::array<int,2>, csl::mp::tuple<float,double>>  // not all same -> ill-formed

    // empty source: output has shape's kind with zero elements
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<int,float>, csl::mp::tuple<>>,             csl::mp::tuple<>>);
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<std::tuple<int,float>,     csl::mp::tuple<>>,             std::tuple<>>);
    // empty shape: output is fully determined by elements_source
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<>,          csl::mp::tuple<int>>,          csl::mp::tuple<int>>);
    // both empty
    static_assert(std::is_same_v<csl::mp::rebind_elements_t<csl::mp::tuple<>,          csl::mp::tuple<>>,             csl::mp::tuple<>>);

    // cvref qualifiers
    static_assert(std::is_same_v<
        csl::mp::rebind_elements_t<
            std::tuple<std::int64_t>,
            csl::mp::tuple<int&,float&&>
        >,
        std::tuple<int&,float&&>
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
namespace test::tuples::algorithm::set {
    using empty = csl::mp::tuple<>;
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<int, double>;

    namespace union_ {

        // empty LHS or RHS => identity
        static_assert(std::is_same_v<
            csl::mp::set_union_t<T0, empty>,
            T0
        >);
        static_assert(std::is_same_v<
            csl::mp::set_union_t<empty, T0>,
            T0
        >);

        // idempotence
        static_assert(std::is_same_v<
            csl::mp::set_union_t<T0, T0>,
            T0
        >);

        // order preservation
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                csl::mp::tuple<int, char>,
                csl::mp::tuple<char, int, float>
            >,
            csl::mp::tuple<int, char, float>
        >);

        // duplicates in RHS
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                csl::mp::tuple<int>,
                csl::mp::tuple<int, int, int>
            >,
            csl::mp::tuple<int>
        >);
        // duplicates in LHS
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                csl::mp::tuple<int, int>,
                csl::mp::tuple<int>
            >,
            csl::mp::tuple<int, int>
        >);

        // cross tuplelikes types
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                std::tuple<int, char>,
                csl::mp::tuple<char, double>
            >,
            std::tuple<int, char, double>
        >);
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                std::tuple<int, char>,
                std::array<char, 2>
            >,
            std::tuple<int, char>
        >);
        static_assert(std::is_same_v<
            csl::mp::set_union_t<
                std::array<char, 2>,
                std::tuple<char>
            >,
            std::array<char, 2>
        >);
    }
    namespace intersection {
        using T0 = csl::mp::tuple<int, char>;
        using T1 = csl::mp::tuple<int, double>;
        
        // empty behavior
        static_assert(std::is_same_v<
            csl::mp::set_intersection_t<T0, empty>,
            empty
        >);
        static_assert(std::is_same_v<
            csl::mp::set_intersection_t<empty, T0>,
            empty
        >);

        // idempotence
        static_assert(std::is_same_v<
            csl::mp::set_intersection_t<T0, T0>,
            T0
        >);

        // order preservation (from LHS!)
        static_assert(std::is_same_v<
            csl::mp::set_intersection_t<
                csl::mp::tuple<int, char, float>,
                csl::mp::tuple<float, int>
            >,
            csl::mp::tuple<int, float>
        >);

        // duplicates in LHS are preserved
        static_assert(std::is_same_v<
            csl::mp::set_intersection_t<
                csl::mp::tuple<int, int, char>,
                csl::mp::tuple<int>
            >,
            csl::mp::tuple<int, int>
        >);
    }
    namespace difference {
    
        // correctness
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<
                csl::mp::tuple<int, int, int>,
                csl::mp::tuple<int, int>
            >,
            csl::mp::tuple<int>
        >);

        // identity
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<T0, empty>,
            T0
        >);

        // annihilation
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<T0, T0>,
            empty
        >);

        // empty lhs
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<empty, T0>,
            empty
        >);

        // order preservation
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<
                csl::mp::tuple<int, char, float>,
                csl::mp::tuple<char>
            >,
            csl::mp::tuple<int, float>
        >);

        // duplicates preserved
        static_assert(std::is_same_v<
            csl::mp::set_difference_t<
                csl::mp::tuple<int, int, char>,
                csl::mp::tuple<int>
            >,
            csl::mp::tuple<int, char>
        >);

        static_assert(std::is_same_v<
            csl::mp::set_difference_t<
                csl::mp::tuple<A,B,D,D,D,G>,
                csl::mp::tuple<B,D,F>
            >,
            csl::mp::tuple<A,D,D,G>
        >);
    }
}
namespace test::tuples::algorithm::unique {

    // ================================================================
    // Invariant helpers: properties that must hold for ANY input
    // ================================================================

    template <csl::mp::concepts::tuple_like T>
    constexpr bool check = csl::mp::is_uniqued_v<csl::mp::unique_t<T>>;

    template <csl::mp::concepts::tuple_like T>
    constexpr bool unchanged = std::is_same_v<
        csl::mp::unique_t<T>,
        csl::mp::unique_t<csl::mp::unique_t<T>>
    >;

    namespace csl_tuple {

        // already uniqued -> no-op
        using already_unique = csl::mp::tuple<int, char, double>;
        static_assert(csl::mp::concepts::support_get_by_type<already_unique>);
        static_assert(csl::mp::concepts::uniqued<already_unique>);
        static_assert(std::is_same_v<csl::mp::unique_t<already_unique>, already_unique>);

        static_assert(not csl::mp::concepts::support_get_by_type<csl::mp::tuple<int, char, int>>);

        // duplicate at end: first occurrence kept
        static_assert(std::is_same_v<
            csl::mp::unique_t<csl::mp::tuple<int, char, double, int>>,
            csl::mp::tuple<int, char, double>
        >);
        // duplicate at start
        static_assert(std::is_same_v<
            csl::mp::unique_t<csl::mp::tuple<int, int, char, double>>,
            csl::mp::tuple<int, char, double>
        >);
        // duplicate in middle
        static_assert(std::is_same_v<
            csl::mp::unique_t<csl::mp::tuple<int, char, int, double>>,
            csl::mp::tuple<int, char, double>
        >);
        // multiple distinct duplicates
        static_assert(std::is_same_v<
            csl::mp::unique_t<csl::mp::tuple<int, char, int, char, double, int>>,
            csl::mp::tuple<int, char, double>
        >);
        // all same type
        static_assert(std::is_same_v<
            csl::mp::unique_t<csl::mp::tuple<int, int, int>>,
            csl::mp::tuple<int>
        >);

        static_assert(unchanged<csl::mp::tuple<>>);
        static_assert(unchanged<csl::mp::tuple<int>>);
        static_assert(check<csl::mp::tuple<int, char, int, char, double, int>>);
        static_assert(unchanged<csl::mp::tuple<int, char, int, char, double, int>>);
        static_assert(unchanged<csl::mp::tuple<int, char, double>>); // already unique
        static_assert(unchanged<csl::mp::tuple<>>);
    }

    namespace std_tuple {

        using already_unique = std::tuple<int, char, double>;
        static_assert(csl::mp::concepts::support_get_by_type<already_unique>);
        static_assert(csl::mp::concepts::uniqued<already_unique>);
        static_assert(std::is_same_v<csl::mp::unique_t<already_unique>, already_unique>);

        static_assert(not csl::mp::concepts::support_get_by_type<std::tuple<int, char, int>>);
        static_assert(std::is_same_v<
            csl::mp::unique_t<std::tuple<int, char, int, char, double, int>>,
            std::tuple<int, char, double>
        >);

        static_assert(check<std::tuple<int, char, int, char, double, int>>);
        static_assert(unchanged<std::tuple<>>);
        static_assert(unchanged<std::tuple<int>>);
        static_assert(unchanged<std::tuple<int, char, int, char, double, int>>);
    }

    namespace std_pair {

        static_assert(csl::mp::concepts::support_get_by_type<std::pair<int, float>>);
        static_assert(csl::mp::concepts::uniqued<std::pair<int, float>>);

        using type = std::pair<int, float>;

        // NOTE: pairs are either same/unchanged, or ill-formed
        static_assert(check<type>);
        static_assert(unchanged<type>);
    }

    namespace std_array {
        // std_array -> std::array<T, N> -> std::array<T, 1>
    }
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

// TODO(Guillaume) forgot some algos here

namespace test::tuples::algorithm::index_of {

    template <csl::mp::concepts::tuple_like tuple_type, typename /*not_found*/>
    constexpr auto not_found = csl::mp::size_v<tuple_type>;

    // csl::mp::tuple - unique types -> use direct storage deduction: O(1), no linear scan.
    namespace optimized_O1 {

        using T = csl::mp::tuple<char, double, float, int>;

        static_assert(csl::mp::index_of_v<T, char>   == 0);
        static_assert(csl::mp::index_of_v<T, double> == 1);
        static_assert(csl::mp::index_of_v<T, float>  == 2);
        static_assert(csl::mp::index_of_v<T, int>    == 3);
        static_assert(csl::mp::index_of_v<T, std::int64_t> == not_found<T, std::int64_t>);

        // index_of == last_index_of
        static_assert(csl::mp::last_index_of_v<T, char>   == 0);
        static_assert(csl::mp::last_index_of_v<T, double> == 1);
        static_assert(csl::mp::last_index_of_v<T, float>  == 2);
        static_assert(csl::mp::last_index_of_v<T, int>    == 3);
        static_assert(csl::mp::last_index_of_v<T, std::int64_t> == not_found<T, std::int64_t>);

        template <std::size_t index>
        constexpr auto check_element(){
            using element_t = csl::mp::element_t<index, T>;
            static_assert(csl::mp::details::concepts::can_deduce_by_type<T, element_t>);
            static_assert(index == csl::mp::index_of_v<T, element_t>);
            static_assert(index == csl::mp::last_index_of_v<T, element_t>);
            // so we implicitly have: static_assert(csl::mp::index_of_v<T, element_t> == csl::mp::last_index_of_v<T, element_t>);
        };
        [[maybe_unused]] constexpr auto check_elements = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((check_element<indexes>()), ...);
            return true;
        }(std::make_index_sequence<csl::mp::size_v<T>>{});
    }

    namespace linear_ON {
        static_assert(not csl::mp::details::concepts::can_deduce_by_type<csl::mp::tuple<int, float, int>, int>);

        using duplicate_at_end = csl::mp::tuple<char, double, float, int, int>;
        static_assert(csl::mp::index_of_v     <duplicate_at_end, int> == 3);
        static_assert(csl::mp::last_index_of_v<duplicate_at_end, int> == 4);

        using duplicate_at_begin = csl::mp::tuple<int, int, float, double>;
        static_assert(csl::mp::index_of_v     <duplicate_at_begin, int> == 0);
        static_assert(csl::mp::last_index_of_v<duplicate_at_begin, int> == 1);

        using duplicate_in_middle = csl::mp::tuple<char, int, int, double>;
        static_assert(csl::mp::index_of_v     <duplicate_in_middle, int> == 1);
        static_assert(csl::mp::last_index_of_v<duplicate_in_middle, int> == 2);

        using duplicate_sparsed = csl::mp::tuple<int, float, int, double, int>;
        static_assert(csl::mp::index_of_v     <duplicate_sparsed, int> == 0);
        static_assert(csl::mp::last_index_of_v<duplicate_sparsed, int> == 4);

        static_assert(csl::mp::index_of_v     <duplicate_at_end, std::int64_t> == not_found<duplicate_at_end, std::int64_t>);
        static_assert(csl::mp::last_index_of_v<duplicate_at_end, std::int64_t> == not_found<duplicate_at_end, std::int64_t>);

        using homogeneous = csl::mp::tuple<int, int, int>;
        static_assert(csl::mp::index_of_v     <homogeneous, int>   == 0);
        static_assert(csl::mp::last_index_of_v<homogeneous, int>   == 2);
        static_assert(csl::mp::index_of_v     <homogeneous, float> == not_found<homogeneous, float>);
    }

    namespace edge_cases {
        // empty: not_found == tuple_size == 0
        using empty_csl_mp_tuple  = csl::mp::tuple<>;
        using empty_std_tuple     = std::tuple<>;
        static_assert(csl::mp::index_of_v     <empty_csl_mp_tuple,  int> == not_found<empty_csl_mp_tuple,  int>);
        static_assert(csl::mp::last_index_of_v<empty_csl_mp_tuple,  int> == not_found<empty_csl_mp_tuple,  int>);
        static_assert(csl::mp::index_of_v     <empty_std_tuple,     int> == not_found<empty_std_tuple,     int>);
        static_assert(csl::mp::last_index_of_v<empty_std_tuple,     int> == not_found<empty_std_tuple,     int>);

        // size == 1: found
        using single_element = csl::mp::tuple<int>;
        static_assert(csl::mp::index_of_v     <single_element, int>   == 0);
        static_assert(csl::mp::last_index_of_v<single_element, int>   == 0);
        // size == 1: not found
        static_assert(csl::mp::index_of_v     <single_element, float> == not_found<single_element, float>);
        static_assert(csl::mp::last_index_of_v<single_element, float> == not_found<single_element, float>);
    }

    // linear
    namespace std_tuple {
        using T = std::tuple<char, double, float, int>;
        static_assert(csl::mp::index_of_v     <T, char>         == 0);
        static_assert(csl::mp::index_of_v     <T, int>          == 3);
        static_assert(csl::mp::index_of_v     <T, std::int64_t> == not_found<T, std::int64_t>);
        static_assert(csl::mp::last_index_of_v<T, int>          == 3);

        using duplicates = std::tuple<int, float, int>;
        static_assert(csl::mp::index_of_v     <duplicates, int> == 0);
        static_assert(csl::mp::last_index_of_v<duplicates, int> == 2);
    }

    // std::array - optimised O(1)
    namespace std_array {
        using T = std::array<int, 4>;
        static_assert(csl::mp::index_of_v     <T, int>   == 0);
        static_assert(csl::mp::last_index_of_v<T, int>   == 3);
        static_assert(csl::mp::index_of_v     <T, float> == not_found<T, float>);
    }

    namespace std_pair {
        using T = std::pair<int, float>;
        static_assert(csl::mp::index_of_v     <T, int>    == 0);
        static_assert(csl::mp::index_of_v     <T, float>  == 1);
        static_assert(csl::mp::index_of_v     <T, double> == not_found<T, double>);
        static_assert(csl::mp::last_index_of_v<T, int>    == 0);
        static_assert(csl::mp::last_index_of_v<T, float>  == 1);

        // Same type in both positions
        using duplicate = std::pair<int, int>;
        static_assert(csl::mp::index_of_v     <duplicate, int>   == 0);
        static_assert(csl::mp::last_index_of_v<duplicate, int>   == 1);
    }
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



