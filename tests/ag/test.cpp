#include <csl/ag.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>
#include <string_view>

// TODO : fix for carrays/function-ptrs
//  const char (&)[7
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

namespace csl::ag::io::details {

    struct indent {
        std::size_t depth;
        friend std::ostream & operator<<(std::ostream & os, const indent & op) {
            return os << std::setw(3 * op.depth) << ""; // NOLINT
        }
    };

    void print_impl(std::ostream & os, auto && value, std::size_t) {
        os << gcl::cx::type_name_v<decltype(value)> << " : " << value << '\n';
    }
    void print_impl(std::ostream & os, csl::ag::concepts::structured_bindable auto && value, std::size_t depth) {

        using value_type = std::remove_cvref_t<decltype(value)>;

        constexpr auto size = []() constexpr { // work-around for ADL issue
            if constexpr (csl::ag::concepts::tuplelike<value_type>)
                return std::tuple_size_v<value_type>;
            else if constexpr (csl::ag::concepts::aggregate<value_type>)
                return csl::ag::size_v<value_type>;
            else
                static_assert(sizeof(value_type) and false, "csl::ag::print : invalid type"); // NOLINT
        }();

        os << gcl::cx::type_name_v<decltype(value)> << " : {\n";

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((
                os << indent{depth + 1} << '[' << indexes << "] ",
                print_impl(os, std::get<indexes>(std::forward<decltype(value)>(value)), depth + 1)
            ), ...);  
        }(std::make_index_sequence<size>{});

        os << indent{depth} << "" << "}\n";
    }
}
//  TODO : std::formatter
namespace csl::ag::io {
    //  For GCC [10.3 .. 12.1] : Can't use the following synthax anymore (constraint depends on itself)
    //  (might be same issue as https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99599)
    //      std::ostream & operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      requires (not ostream_shiftable<decltype(value)>::value)
    //  nor delayed adl :
    //      std::ostream & details::ostream_shift(std::ostream & os, csl::ag::concepts::aggregate auto const& value)
    //      requires (not ostream_shiftable<decltype(value)>::value); // never defined
    //
    //      auto operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      -> decltype(details::ostream_shift(os, value))
    //
    // Warning : csl::ag makes aggregate type tuplelike
    auto & operator<<(std::ostream & os, csl::ag::concepts::aggregate auto const & value)
    requires (not std::is_array_v<std::remove_cvref_t<decltype(value)>>) // temporary quickfix
    {
        details::print_impl(os, std::forward<decltype(value)>(value), 0);
        return os;
    }

    // WIP : indent
    // todo : stack/queue : depth -> count
    struct indent_type {
        std::ostream & os;
        std::size_t depth;

        auto & operator<<(const auto & value) {
            // os << std::setw(depth * 3) << "" << value;
            return *this;
        }
    };

    struct indent {
        struct increase{};
        struct decrease{};
        std::size_t depth = 0;
    };
    auto operator<<(std::ostream & os, indent && value) {
        return indent_type{ os, value.depth };
    }
    auto & operator<<(indent_type & value, indent::increase) {
        value.depth += 1;
        return value;
    }
}

struct type_0{ int i = 0; char c = 'a'; };
struct type_1{ char c = 'c'; type_0 t; };
struct type_2{ bool b = true; type_1 t; std::tuple<int, char> tu = { 2, 'b'}; std::array<char, 3> a = {'a', 'b', 'c'}; std::pair<int, int> p = { 42, 43 }; };
struct type_3{ int i = 0; char c = 'a'; };

auto & operator<<(std::ostream & os, type_3) { return os << "type_3 : user-defined operator<<(std::ostream&, const T &)\n"; }

auto main() -> int {

    using namespace csl::ag::io;

    std::cout
        << type_0{} << "-----\n"
        << type_1{} << "-----\n"
        << type_2{} << "-----\n"
        << type_3{} << "-----\n"
        // << std::tuple{42, 'a'} << "-----\n"
    ;

    // {
    //     auto value = toto{ 42, 'a' };
    //     [[maybe_unused]] auto && [ v0, v1 ] = value;
    //     print(value);
    // }

    // auto value = toto{ 42, 'A' }; // NOLINT
    // auto as_tuple = csl::ag::as_tuple(value); // WTF not a constant expression ???

    // static_assert(std::same_as<
    //     int&,
    //     std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    // >);
    // static_assert(std::same_as<
    //     char&,
    //     std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    // >);

    // // /*static_*/assert(csl::ag::get<0>(value) == 42);
    // // /*static_*/assert(csl::ag::get<1>(value) == 'A');

    // assert(
    //     std::addressof(std::get<0>(as_tuple)) ==
    //     std::addressof(value.i)
    // );
    // assert(
    //     std::addressof(std::get<1>(as_tuple)) ==
    //     std::addressof(value.c)
    // );

    // // // print(value);

    // struct qwe{ int i; char & c; bool && b; };
    // // static_assert(std::tuple_size<qwe>::value == 3);
    // static_assert(std::same_as<
    //     int,
    //     csl::ag::element<0, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     char &,
    //     csl::ag::element<1, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     bool &&,
    //     csl::ag::element<2, qwe>::type
    // >);
}
