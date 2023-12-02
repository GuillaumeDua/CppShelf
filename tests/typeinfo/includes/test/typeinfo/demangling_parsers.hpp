#pragma once

#include <csl/typeinfo.hpp>
#include <array>
#include <ranges>
#include <algorithm>
#include <string_view>

#define fwd(arg) std::forward<decltype(arg)>(arg)

// --- impl ---
struct short_name_tag{};
constexpr static auto parse_mangling(std::string_view value, short_name_tag){

    constexpr static auto namespace_token = std::string_view{"::"}; // todo: in details ns
    
    const auto seq_begin = [&value] -> std::string_view::size_type {
        const auto first_double_colons_pos = value.find(namespace_token);
        if (first_double_colons_pos == std::string_view::npos)
            return 0;
        const auto first_rspace = value.rfind(' ', first_double_colons_pos);
        return first_rspace == std::string_view::npos
            ? std::string_view::size_type{0}
            : first_rspace + 1
        ;
    }();
    const auto last_namespace_token = value.rfind(namespace_token);
    const auto seq_end = last_namespace_token == std::string_view::npos
        ? std::string_view::npos
        : last_namespace_token + 1
    ;
    return value.substr(seq_begin, seq_end);
}
// ------------

template <typename T>
[[nodiscard]] constexpr static auto make_array(auto && ... args)
noexcept ((std::is_nothrow_constructible_v<T, decltype(args)> and ...))
requires (std::constructible_from<T, decltype(args)> and ...)
{
    using result_type = std::array<T, sizeof...(args)>;
    return result_type{ fwd(args)... };
}
namespace portability::ranges::views {
    // naive/poor/bad implementation of std::ranges::views::zip to enhance support/portability
    template <std::ranges::range T, std::ranges::random_access_range ... Us>
    constexpr auto zip(const T & lhs, const Us & ... rhs)
    {
        if ((false or ... or (std::ranges::size(lhs) not_eq std::ranges::size(rhs))))
            throw std::invalid_argument{"portability::ranges::views::zip: requires equal size"};

        using value_type = std::tuple<
            const typename T::value_type &,
            const typename Us::value_type &...
        >;

        return lhs | std::ranges::views::transform([&, index = std::size_t{0}](const auto & value) mutable {
            auto result = value_type{
                value,
                rhs.at(index)...
            };
            index = ++index % std::ranges::size(lhs); // :shrug:
            return result;
        });
    }
}

constexpr auto function_names = make_array<std::string_view>(
// GCC-13.2, functions
    "void A::B::C::DD::EEE::FFFF::func(int) [with T = void]",
    "static void A::B::C::DD::EEE::FFFF::user_defined<T>::static_func(int) [with U = void; T = void]",
    "void A::B::C::DD::EEE::FFFF::user_defined<T>::func(int) const && [with U = void; T = void]"
    "A::B::C::DD::EEE::FFFF::user_defined<>::func<>(int) const &&::<lambda(int)> [with asd = void]"
// Clang-17.0.1, functions
    "void A::B::C::DD::EEE::FFFF::func(int) [T = void]",
    "static void A::B::C::DD::EEE::FFFF::user_defined<>::static_func(int) [T = void, U = void]"
    "void A::B::C::DD::EEE::FFFF::user_defined<>::func(int) const && [T = void, U = void]",
    "auto A::B::C::DD::EEE::FFFF::user_defined<>::func(int)::(anonymous class)::operator()(int) const [T = void, asd = void]"
);


#include <iostream>
namespace test::typeinfo {
    void function_shortnames(){
        constexpr auto function_shortnames_expected = make_array<std::string_view>(
        // GCC-13.2, functions
            "A::B::C::DD::EEE::FFFF::func",
            "A::B::C::DD::EEE::FFFF::user_defined<T>::static_func",
            "A::B::C::DD::EEE::FFFF::user_defined<T>::func",
            "A::B::C::DD::EEE::FFFF::user_defined<>::func<>(int) const &&::<lambda(int)>",
        // Clang-17.0.1, functions
            "A::B::C::DD::EEE::FFFF::func",
            "A::B::C::DD::EEE::FFFF::user_defined<>::static_func",
            "A::B::C::DD::EEE::FFFF::user_defined<>::func",
            "A::B::C::DD::EEE::FFFF::user_defined<>::func(int)::(anonymous class)::operator()"
        );

        using test_result = struct test_result_t {
            std::string_view input, expected, value;
            constexpr operator bool() const noexcept { return expected == value; }
        };

        // TODO(Guss): make sure zipped are the same std::ranges::size
        // auto failed_tests = std::ranges::views::zip(function_names, function_shortnames_expected)
        auto failed_tests = portability::ranges::views::zip(function_names, function_shortnames_expected)
            | std::ranges::views::transform([](const auto & element) -> test_result {
                const auto & [value, expected] = element;
                return {
                    value,
                    expected,
                    parse_mangling(value, short_name_tag{})
                };
            })
            | std::ranges::views::filter([](const auto & result) -> bool {
                return not result;
            })
        ;

        for (const auto & value: failed_tests) {
            const auto & [input, expected, result] = value;
            std::cerr
                << "input    = [" << input << "]\n"
                << "expected = [" << expected << "]\n"
                << "result   = [" << result << "]\n\n"
            ;
        }
    }
}

// WIP: https://godbolt.org/z/Wb36Md8Yx
