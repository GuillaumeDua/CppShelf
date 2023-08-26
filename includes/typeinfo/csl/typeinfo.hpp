#pragma once

#if not __cplusplus >= 201703L
# error "csl/typeinfo.hpp requires C++17"
#endif

// TODO(Guillaume): refactor type_name, value_name when universal template parameter are available

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace csl::typeinfo::details
{
    struct type_prefix_tag {
        constexpr static std::string_view value = "T = ";
    };
    struct value_prefix_tag {
        constexpr static std::string_view value = "value = ";
    };

    template <typename prefix_tag_t>
    static constexpr auto parse_mangling(std::string_view value, std::string_view function)
    {
        value.remove_prefix(value.find(function) + std::size(function));
#if defined(__GNUC__) or defined(__clang__)
        value.remove_prefix(value.find(prefix_tag_t::value) + std::size(prefix_tag_t::value));
#if defined(__clang__)
        value.remove_suffix(value.length() - value.rfind(']'));
#elif defined(__GNUC__) // GCC
        value.remove_suffix(value.length() - value.find(';'));
#endif
#elif defined(_MSC_VER)
        value.remove_prefix(value.find('<') + 1);
        if (auto enum_token_pos = value.find("enum "); enum_token_pos == 0)
            value.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        value.remove_suffix(value.length() - value.rfind(">(void)"));
#endif
        return value;
    }
}

namespace csl::typeinfo
{   // constexpr typeinfo that does not relies on __cpp_rtti
    // 
    // Warning: Produced outputs ARE NOT portable: inconsistencies exist across compilers (GCC, Clang, msvc-cl)
    //
    // Known limitations :
    //  type_name : type aliases
    //      ex : std::string
    //          std::basic_string<char>
    //          std::__cxx11::basic_string<char>
    //          std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>
    //
    //  value_name : values representation
    //      ex : int(42)
    //          0x2a ont MsVC/CL
    //      use <charconv> std::to_chars into std::string_view for reliable basic numerical values

    template <typename T>
    static constexpr /*consteval*/ auto type_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::type_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::type_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "csl::typeinfo : unhandled plateform");
#endif
    }
    template <typename T>
    constexpr static auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr auto type_name(/*no parameters allowed*/) -> std::string_view
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr auto value_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::value_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::value_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "csl::typeinfo : unhandled plateform");
#endif
    }
    template <auto value>
    constexpr inline auto value_name_v = value_name<value>();

    // TODO(Guss): hash_code, see #93 https://github.com/GuillaumeDua/CppShelf/issues/93
    //  - update example accordingly
}
