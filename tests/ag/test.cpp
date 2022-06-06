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

void print_impl(auto && value, std::size_t depth) {
    std::cout
        << std::setw(3 * depth) << ""
        << gcl::cx::type_name_v<decltype(value)> << " : " << value << '\n'
    ;
}
// TODO : unqualified get : csl::ag, std::(tuple_interface), etc.
//  agnostic from csl::ag, std, etc. : it's the user responsibility to provide adequats ns to lookup to ?
void print_impl(csl::ag::concepts::aggregate auto && value, std::size_t depth) {

    std::cout
        << std::setw(3 * depth) << ""
        << gcl::cx::type_name_v<decltype(value)> << " : {\n"
    ;

    auto as_tuple = csl::ag::as_tuple(std::forward<decltype(value)>(value));
    using as_tuple_t = std::remove_cvref_t<decltype(as_tuple)>;
    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((
            print_impl(std::get<indexes>(std::forward<decltype(as_tuple)>(as_tuple)), depth + 1)
        ), ...);  
    }(std::make_index_sequence<std::tuple_size_v<as_tuple_t>>{});

    std::cout
        << std::setw(3 * depth) << ""
        << "}\n"
    ;
}


// interface
// TODO : default std::formatter
void print(csl::ag::concepts::aggregate auto && value) {
    print_impl(std::forward<decltype(value)>(value), 0);
}

struct toto{ int i; char c; };
struct titi{ char c = 'c'; toto t = { 1, 'a'}; };
struct tata{ bool b = true; titi t; };

auto main() -> int {
    using namespace csl::ag;

    print(titi{});
    print(tata{});

    // TODO : wrap that in a test function

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

    /*static_*/assert(csl::ag::get<0>(value) == 42);
    /*static_*/assert(csl::ag::get<1>(value) == 'A');

    assert(
        std::addressof(std::get<0>(as_tuple)) ==
        std::addressof(value.i)
    );
    assert(
        std::addressof(std::get<1>(as_tuple)) ==
        std::addressof(value.c)
    );

    print(value);

    struct qwe{ int i; char & c; bool && b; };
    static_assert(csl::ag::size<qwe>::value == 3);
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
