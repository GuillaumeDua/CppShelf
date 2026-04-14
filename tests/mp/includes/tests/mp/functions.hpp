#pragma once

#include <csl/mp.hpp>
#include <string>
#include <string_view>

// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::function::factory::tie {

    constexpr int i{};
    constexpr char c{};
    constexpr auto tied = csl::mp::functions::tie(i, c);

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
        auto value = csl::mp::functions::make_tuple(i, c, std::move(i)); // NOLINT(*-move-const-arg)

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
        auto value = csl::mp::functions::forward_as_tuple(i, c, std::move(i)); // NOLINT(*-move-const-arg)

        static_assert(std::same_as<
            decltype(value),
            csl::mp::tuple<int &, const char&, int&&>
        >);
    }
}
namespace test::function::factory::cat {
    
    static_assert(
        csl::mp::functions::cat(
            std::tuple{},
            csl::mp::tuple{}
        ) == csl::mp::tuple{}
    );

    static_assert(
        csl::mp::functions::cat(
            std::tuple{ 1, 2 },
            csl::mp::tuple{ '3' },
            std::array<double, 1>{ 4.F },
            std::pair{5,6}
        ) == csl::mp::tuple{1,2, '3', 4.F, 5,6}
    );
}
namespace test::function::factory::cat_result {
    using empty = csl::mp::tuple<>;

    static_assert(std::same_as<empty, csl::mp::type_traits::cat_result_t<>>);
    static_assert(std::same_as<empty, csl::mp::type_traits::cat_result_t<empty>>);
    static_assert(std::same_as<empty, csl::mp::type_traits::cat_result_t<empty, empty>>);

    static_assert(std::same_as<
        csl::mp::tuple<int, char, double>,
        csl::mp::type_traits::cat_result_t<
            csl::mp::tuple<int>,
            csl::mp::tuple<char, double>
        >
    >);
    static_assert(std::same_as<
        csl::mp::tuple<int, char, double, bool>,
        csl::mp::type_traits::cat_result_t<
            csl::mp::tuple<int>,
            std::pair<char, double>,
            std::tuple<bool>
        >
    >);

    
}
namespace test::tuples::function::for_each::concepts {

    struct A{ constexpr A(bool){} }; // NOLINT(*-explicit-constructor)
    struct B{};

    struct f {
        constexpr auto operator()(A value){ return value; }
        constexpr auto operator()(B value) noexcept { return value; }
    };

    static_assert(csl::mp::concepts::can_for_each<csl::mp::tuple<B, A, bool>, f>);
    static_assert(csl::mp::concepts::can_for_each<std::tuple<B, A, bool>, f>);
    static_assert(csl::mp::concepts::can_for_each<std::pair<B, A>, f>);
    static_assert(csl::mp::concepts::can_for_each<std::array<B, 2>, f>);
    static_assert(not csl::mp::concepts::can_for_each<std::pair<B, std::string>, f>); // no conversion from std::string to B

    static_assert(not csl::mp::concepts::can_nothrow_for_each<csl::mp::tuple<B, A, bool>, f>);
    static_assert(not csl::mp::concepts::can_nothrow_for_each<std::tuple<B, A, bool>, f>);
    static_assert(not csl::mp::concepts::can_nothrow_for_each<std::pair<B, A>, f>);
    static_assert(csl::mp::concepts::can_nothrow_for_each<std::pair<B, B>, f>);
    static_assert(csl::mp::concepts::can_nothrow_for_each<std::array<B, 2>, f>);
}

namespace test::tuples::function::for_each {

    constexpr auto my_reduce(csl::mp::concepts::tuple_like auto && values){
        int reduced{};
        csl::mp::functions::for_each(values,[&reduced](const auto & value){ reduced += value; });
        return reduced;
    }

    constexpr auto expected = 6;
    static_assert(expected == my_reduce(csl::mp::tuple{1,2,3}));
    static_assert(expected == my_reduce(std::tuple{1,2,3}));
    static_assert(expected == my_reduce(std::array{1,2,3}));
    static_assert(expected == my_reduce(std::pair{2,4}));

    namespace result {

        using tuple_int = csl::mp::tuple<int>;
        using f_tuple_int = decltype([](int){});

        static_assert(std::is_same_v<
            f_tuple_int,
            csl::mp::type_traits::for_each_result_t<
                tuple_int,
                f_tuple_int
            >
        >);
    }
}
namespace test::tuples::function::for_each_enumerate {

    struct result_type {
        std::size_t index{};
        int value{};

        constexpr bool operator==(const result_type &) const noexcept = default;
    };
    constexpr auto my_indexed_reduce(csl::mp::concepts::tuple_like auto && values){
        result_type result{};
        csl::mp::functions::for_each_enumerate(
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

    namespace result {

        using tuple_int = csl::mp::tuple<int>;
        using f_tuple_int = decltype([](std::size_t, int){});

        static_assert(std::is_same_v<
            f_tuple_int,
            csl::mp::type_traits::for_each_enumerate_result_t<
                tuple_int,
                f_tuple_int
            >
        >);
    }
}

namespace test::tuples::function::for_each_enumerate_nttp::concepts {
    
    struct A{};
    struct B{};

    struct functor      {
        template <std::size_t> constexpr void operator()(A){}
    };
    template <> constexpr void functor::operator()<2>(A) = delete; // NOTE: GCC(13, 14) does not support explicit specialization in non-namespace scope

    struct functor_auto { template <std::size_t> constexpr void operator()(auto){} };
    template <std::size_t> constexpr void f(auto){};
    constexpr auto adapted_f = []<std::size_t i>(auto value){ return f<i>(value); };

    static_assert(csl::mp::concepts::can_for_each_enumerate_nttp<std::tuple<A>, functor>);
    static_assert(csl::mp::concepts::can_for_each_enumerate_nttp<std::tuple<A>, functor_auto>);
    static_assert(csl::mp::concepts::can_for_each_enumerate_nttp<std::tuple<int>, decltype(adapted_f)>);

    struct functor_noexcept {
        template <std::size_t> constexpr void operator()(A) noexcept {}
        template <std::size_t> constexpr void operator()(B) {}
    };

    static_assert(not csl::mp::concepts::can_nothrow_for_each_enumerate_nttp<std::tuple<A>, functor>);
    static_assert(csl::mp::concepts::can_nothrow_for_each_enumerate_nttp<std::tuple<A>, functor_noexcept>);
    static_assert(not csl::mp::concepts::can_nothrow_for_each_enumerate_nttp<std::tuple<B>, functor_noexcept>);
    static_assert(not csl::mp::concepts::can_nothrow_for_each_enumerate_nttp<std::tuple<A, B>, functor_noexcept>);
}

namespace test::tuples::function::for_each_enumerate_nttp {
    struct result_type {
        std::size_t index{};
        int value{};

        constexpr bool operator==(const result_type &) const noexcept = default;
    };
    constexpr auto my_indexed_reduce(csl::mp::concepts::tuple_like auto && values){
        result_type result{};
        csl::mp::functions::for_each_enumerate_nttp(
            values,
             [&result]<std::size_t i>(const auto & value) {
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

namespace test::tuples::function::apply::concepts {

    using throw_f_t = decltype([](int, int){});
    using nothrow_f_t = decltype([](int, int) noexcept {});

    // can_apply
    static_assert(csl::mp::concepts::can_apply<csl::mp::tuple<int, int>, throw_f_t>);
    static_assert(csl::mp::concepts::can_apply<std::tuple<int, int>, throw_f_t>);
    static_assert(csl::mp::concepts::can_apply<std::array<int, 2>, throw_f_t>);

    // can_nothrow_apply
    static_assert(not csl::mp::concepts::can_nothrow_apply<csl::mp::tuple<int, int>, throw_f_t>);
    static_assert(not csl::mp::concepts::can_nothrow_apply<std::tuple<int, int>, throw_f_t>);
    static_assert(not csl::mp::concepts::can_nothrow_apply<std::array<int, 2>, throw_f_t>);
    static_assert(csl::mp::concepts::can_nothrow_apply<csl::mp::tuple<int, int>, nothrow_f_t>);
    static_assert(csl::mp::concepts::can_nothrow_apply<std::tuple<int, int>, nothrow_f_t>);
    static_assert(csl::mp::concepts::can_nothrow_apply<std::array<int, 2>, nothrow_f_t>);
}
namespace test::tuples::function::apply {

    constexpr auto my_reduce = [](auto ... values){ return (0 + ... + values); };

    constexpr auto expected = 6;
    static_assert(expected == csl::mp::functions::apply(csl::mp::tuple{1,2,3}, my_reduce));
    static_assert(expected == csl::mp::functions::apply(std::tuple{1,2,3}, my_reduce));
    static_assert(expected == csl::mp::functions::apply(std::array{1,2,3}, my_reduce));
    static_assert(expected == csl::mp::functions::apply(std::pair{2,4}, my_reduce));
}
namespace test::tuples::function::apply::result {

    struct F {
        constexpr int  operator()(int, int  value){ return value ; };
        constexpr char operator()(int, char value){ return value ; };
        constexpr bool operator()(int, bool value){ return value ; };
    };

    static_assert(std::is_same_v<
        int,
        csl::mp::type_traits::apply_result_t<csl::mp::tuple<int, int>, F>>
    );
    static_assert(std::is_same_v<
        char,
        csl::mp::type_traits::apply_result_t<std::tuple<int, char>, F>>
    );
    static_assert(std::is_same_v<
        bool,
        csl::mp::type_traits::apply_result_t<std::pair<int, bool>, F>>
    );
}

namespace test::tuples::algorithm::fold::homogeneous {

    // empty: both directions always return init, regardless of operation
    static_assert(csl::mp::functions::fold_left (csl::mp::tuple{}, std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::functions::fold_left (std::tuple{},     std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::functions::fold_right(csl::mp::tuple{}, std::minus<void>{}, 0) == 0);
    static_assert(csl::mp::functions::fold_right(std::tuple{},     std::minus<void>{}, 0) == 0);

    // minus:
    //  single:     fold_left  = f(init, x) = 0 - 5 = -5
    //              fold_right = f(x, init) = 5 - 0 =  5
    //  multiple:   fold_left ({1,2,3}, -, 0) = ((0-1)-2)-3 = -6
    //              fold_right({1,2,3}, -, 0) =  1-(2-(3-0)) = 2
    static_assert(csl::mp::functions::fold_left (csl::mp::tuple{5},   std::minus<void>{}, 0)   == -5);
    static_assert(csl::mp::functions::fold_left (std::tuple{5},       std::minus<void>{}, 0)   == -5);
    static_assert(csl::mp::functions::fold_right(csl::mp::tuple{5},   std::minus<void>{}, 0)   ==  5);
    static_assert(csl::mp::functions::fold_right(std::tuple{5},       std::minus<void>{}, 0)   ==  5);
    static_assert(csl::mp::functions::fold_left (std::array{1, 2, 3}, std::minus<void>{}, int{}) == -6);
    static_assert(csl::mp::functions::fold_right(std::array{1, 2, 3}, std::minus<void>{}, int{}) ==  2);

    // plus:
    static_assert(csl::mp::functions::fold_left (std::array{0,1,2,3,4,5}, std::plus<void>{}, int{}) == 15); // NOLINT(*-magic-numbers)
    static_assert(csl::mp::functions::fold_right(std::array{0,1,2,3,4,5}, std::plus<void>{}, int{}) == 15); // NOLINT(*-magic-numbers)

    // result_type
    static_assert(std::is_same_v<csl::mp::type_traits::fold_left_result_t <std::array<int,2>, std::plus<void>,  int>, int>);
    static_assert(std::is_same_v<csl::mp::type_traits::fold_right_result_t<std::array<int,2>, std::plus<void>,  int>, int>);
    static_assert(std::is_same_v<csl::mp::type_traits::fold_left_result_t <std::array<int,2>, std::minus<void>, int>, int>);
    static_assert(std::is_same_v<csl::mp::type_traits::fold_right_result_t<std::array<int,2>, std::minus<void>, int>, int>);
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
        decltype(csl::mp::functions::fold_left(value, std::plus<void>{}, std::string{})),
        csl::mp::type_traits::fold_left_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::is_same_v<
        csl::mp::type_traits::fold_left_result_t<decltype(value), std::plus<void>, std::string>,
        std::string
    >);
    static_assert(csl::mp::functions::fold_left(value, std::plus<void>{}, std::string{}) == "abcdefg");

    static_assert(std::is_same_v<
        decltype(csl::mp::functions::fold_right(value, std::plus<void>{}, std::string{})),
        csl::mp::type_traits::fold_right_result_t<decltype(value), std::plus<void>, std::string>
    >);
    static_assert(std::is_same_v<
        csl::mp::type_traits::fold_right_result_t<decltype(value), std::plus<void>, std::string>,
        std::string
    >);
    static_assert(csl::mp::functions::fold_right(value, std::plus<void>{}, std::string{}) == "abcdefg");
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

        static_assert(csl::mp::functions::fold_left( value, expression_accumulator, "_"s) == "(((_,x),y),z)");
        static_assert(csl::mp::functions::fold_right(value, expression_accumulator, "_"s) == "(x,(y,(z,_)))");
    #endif
}
namespace test::tuples::algorithm::functions::all_any_none_of {

    namespace csl_tuple {
        constexpr auto value = csl::mp::tuple{ 42, .42f, 'a' };
        static_assert(csl::mp::functions::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::functions::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::functions::none_of(value, [](const auto & element){ return element < 0; }));

        namespace with_rvalue {
            static_assert(csl::mp::functions::all_of(csl::mp::tuple{ 42, .42f, 'a' }, [](const auto & element){ return element > 0; }));
            static_assert(csl::mp::functions::all_of(csl::mp::functions::make_tuple( 42, .42f, 'a' ), [](const auto & element){ return element > 0; }));
            static_assert(csl::mp::functions::all_of(csl::mp::functions::forward_as_tuple( 42, .42f, 'a' ), [](const auto & element){ return element > 0; }));
        }
    }
    namespace std_tuple {
        constexpr auto value = std::tuple{ 42, .42f, 'a' };
        static_assert(csl::mp::functions::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::functions::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::functions::none_of(value, [](const auto & element){ return element < 0; }));
    }
    namespace std_array {
        constexpr auto value = std::array{ 42.F, .42F };
        static_assert(csl::mp::functions::all_of(value, [](const auto & element){ return element > 0; }));
        static_assert(csl::mp::functions::any_of(value, [](const auto & element){ return element < 1; }));
        static_assert(csl::mp::functions::none_of(value, [](const auto & element){ return element < 0; }));
    }
}

// NOLINTEND(*-avoid-magic-numbers)
