// available on godbolt: https://godbolt.org/z/ssEW9ferd

#include <csl/functional.hpp>

// functors
auto func(int, char) -> bool { return {}; }
const auto lambda = [](int, char) -> bool { return {}; };
const auto std_function = std::function{ lambda };
// non-static member functions
struct type { auto mem_func(int, char) -> bool { return {}; } };

// demo
auto main(int, char*argv[]) -> int {
    using namespace csl::functional;

    // function trait: functors
    const auto test_function = []<concepts::simple_callable F>{
        using trait = function_trait<F>;
        static_assert(std::is_same_v<
            arguments<int, char>,
            typename trait::arguments_type
        >);
        static_assert(std::is_same_v<bool, typename trait::result_type>);
        static_assert(type_traits::is_invocable_v<F, arguments<int, char>>);
    };
    const auto test_each_function = [test_function]<concepts::simple_callable ... Fs>(){
        ((test_function.template operator()<Fs>()), ...);
    };

    test_each_function.template operator()<
        decltype(func),
        decltype(lambda),
        decltype(std_function)
    >();

    // function trait: non static member functions
    static_assert(std::is_same_v<
        function_trait<decltype(&type::mem_func)>::arguments_type,
        arguments<type, int, char>
    >);

    // overload
    auto overloaded = overload {
        []() -> bool { return {}; },
        [](int) -> float { return {}; },
        [](char, bool) -> double { return {}; }
    };
    using trait = overload_trait_t<decltype(overloaded)>;
    // overload: argument types
    static_assert(std::is_same_v<
        typename std::tuple_element_t<0, trait>::arguments_type,
        arguments<>
    >);
    static_assert(std::is_same_v<
        typename std::tuple_element_t<1, trait>::arguments_type,
        arguments<int>
    >);
    static_assert(std::is_same_v<
        typename std::tuple_element_t<2, trait>::arguments_type,
        arguments<char, bool>
    >);
    // overload: result types
    static_assert(std::is_same_v<bool,   typename std::tuple_element_t<0, trait>::result_type>);
    static_assert(std::is_same_v<float,  typename std::tuple_element_t<1, trait>::result_type>);
    static_assert(std::is_same_v<double, typename std::tuple_element_t<2, trait>::result_type>);
}