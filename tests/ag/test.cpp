#include <csl/ag.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>
#include <string_view>

namespace gcl::cx {
    template <typename T>
    static constexpr /*consteval*/ auto type_name(/*no parameters allowed*/)
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        constexpr std::string_view prefix = "T = ";
        str_view.remove_prefix(str_view.find(prefix) + prefix.length());
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
    #elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
        return str_view;
    }
    template <typename T>
    constexpr inline auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr auto type_name(/*no parameters allowed*/)
    -> std::string_view
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr auto value_name(/*no parameters allowed*/)
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        constexpr std::string_view prefix = "value = ";
        str_view.remove_prefix(str_view.find(prefix) + prefix.length());
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
    #elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
        return str_view;
    }
    template <auto value>
    constexpr inline auto value_name_v = value_name<value>();
}

template <typename T>
struct is_std_array : std::false_type{};
template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type{};

void print_impl(auto && value, std::size_t depth) {
    std::cout
        << gcl::cx::type_name_v<decltype(value)> << " : " << value << '\n'
    ;
}
void print_impl(csl::ag::concepts::tuplelike auto && value, std::size_t depth) {

    using value_type = std::remove_cvref_t<decltype(value)>;

    std::cout << gcl::cx::type_name_v<decltype(value)> << " : {\n";

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((
            std::cout << std::setw(3 * (depth + 1)) << "" << '[' << indexes << "] ",
            print_impl(std::get<indexes>(std::forward<decltype(value)>(value)), depth + 1)
        ), ...);  
    }(std::make_index_sequence<std::tuple_size<value_type>::value>{});

    std::cout
        << std::setw(3 * depth) << ""
        << "}\n"
    ;
}
void print_impl(csl::ag::concepts::aggregate auto && value, std::size_t depth)
requires (not is_std_array<std::remove_cvref_t<decltype(value)>>::value)
{
    using value_type = std::remove_cvref_t<decltype(value)>;

    std::cout << gcl::cx::type_name_v<decltype(value)> << " : {\n";

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        using csl::ag::get;
        ((
            std::cout << std::setw(3 * (depth + 1)) << "" << '[' << indexes << "] ",
            print_impl(get<indexes>(std::forward<decltype(value)>(value)), depth + 1)
        ), ...);  
    }(std::make_index_sequence<csl::ag::tuple_size<value_type>::value>{});

    std::cout
        << std::setw(3 * depth) << ""
        << "}\n"
    ;
}

// void print_impl(csl::ag::concepts::tuplelike auto && value, std::size_t depth) {
//
//     using value_type = std::remove_cvref_t<decltype(value)>;
//     std::cout << gcl::cx::type_name_v<decltype(value)> << " : {\n";
//     // if constexpr (csl::ag::concepts::aggregate<value_type>) {
//     // using tuple_size = std::conditional<
//     //     csl::ag::concepts::aggregate<value_type>,
//     //     csl::ag::tuple_size
//     // >;
//     // using namespace csl::ag;
//
//         // using tuple_size = typename csl::ag::tuple_size<value_type>;
//         [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
//             using std::get;
//             ((
//                 std::cout << std::setw(3 * (depth + 1)) << "" << '[' << indexes << "] ",
//                 print_impl(get<indexes>(std::forward<decltype(value)>(value)), depth + 1)
//             ), ...);  
//         }(std::make_index_sequence<std::tuple_size<value_type>::value>{});
//
//     std::cout
//         << std::setw(3 * depth) << ""
//         << "}\n"
//     ;
// }


// interface
// TODO : default std::formatter
void print(csl::ag::concepts::aggregate auto && value) {
    print_impl(std::forward<decltype(value)>(value), 0);
}

struct toto{ int i = 0; char c = 'a'; };
struct titi{ char c = 'c'; toto t; };
struct tata{ bool b = true; titi t; std::tuple<int, char> tu = { 2, 'b'}; std::array<char, 3> a = {'a', 'b', 'c'}; std::pair<int, int> p = { 42, 43 }; };

auto main() -> int {

    std::puts("-------");
    print(toto{});
    std::puts("-------");
    print(titi{});
    std::puts("-------");
    print(tata{});
    std::puts("-------");

    {
        auto value = toto{ 42, 'a' };
        [[maybe_unused]] auto && [ v0, v1 ] = value;
        print(value);
    }

    auto value = toto{ 42, 'A' }; // NOLINT
    auto as_tuple = csl::ag::as_tuple(value); // WTF not a constant expression ???

    static_assert(std::same_as<
        int&,
        std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    >);
    static_assert(std::same_as<
        char&,
        std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    >);

    // /*static_*/assert(csl::ag::get<0>(value) == 42);
    // /*static_*/assert(csl::ag::get<1>(value) == 'A');

    assert(
        std::addressof(std::get<0>(as_tuple)) ==
        std::addressof(value.i)
    );
    assert(
        std::addressof(std::get<1>(as_tuple)) ==
        std::addressof(value.c)
    );

    // // print(value);

    struct qwe{ int i; char & c; bool && b; };
    // static_assert(std::tuple_size<qwe>::value == 3);
    static_assert(std::same_as<
        int,
        csl::ag::element<0, qwe>::type
    >);
    static_assert(std::same_as<
        char &,
        csl::ag::element<1, qwe>::type
    >);
    static_assert(std::same_as<
        bool &&,
        csl::ag::element<2, qwe>::type
    >);
}
