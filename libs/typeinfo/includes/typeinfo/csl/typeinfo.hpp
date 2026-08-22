#pragma once

// NOTE: Detectable by other csl libraries (via #if defined(...)) to know csl::typeinfo is available, even when it wasn't reachable through __has_include.
//  e.g. when a user manually #include's this header via a direct URL (such as on Compiler Explorer / Godbolt), where <csl/typeinfo.hpp> isn't a resolvable path.
#define CSL_TYPEINFO__INCLUDED

#if not __cplusplus >= 201703L
# error "csl/typeinfo.hpp requires C++17"
#endif

// TODO(Guillaume): refactor type_name, value_name when universal template parameter are available

#include <string_view>
#include <array>
#include <cstddef>

namespace csl::typeinfo::details
{
    struct type_prefix_tag {
        constexpr static std::string_view value = "T = ";
    };
    struct value_prefix_tag {
        constexpr static std::string_view value = "value = ";
    };

    template <typename prefix_tag_t>
    constexpr static auto parse_mangling(std::string_view value, std::string_view function) -> std::string_view
    {
        value.remove_prefix(value.find(function) + std::size(function));
#if defined(__GNUC__) or defined(__clang__)
        value.remove_prefix(value.find(prefix_tag_t::value) + std::size(prefix_tag_t::value));
# if defined(__clang__)
        value.remove_suffix(value.length() - value.rfind(']'));
# elif defined(__GNUC__) // GCC
        value.remove_suffix(value.length() - value.find(';'));
# endif
#elif defined(_MSC_VER)
        value.remove_prefix(value.find('<') + 1);
        // WARNING(limitation, inconsistency) This should remove all occurences, not just the first.
        // if (auto pos = value.find("enum ");   pos == 0) value.remove_prefix(pos + sizeof("enum ")   - 1);
        // if (auto pos = value.find("struct "); pos == 0) value.remove_prefix(pos + sizeof("struct ") - 1);
        // if (auto pos = value.find("class ");  pos == 0) value.remove_prefix(pos + sizeof("class ")  - 1);
        value.remove_suffix(value.length() - value.rfind(">(void)"));
#endif
        return value;
    }

    template <typename T>
    [[nodiscard]] constexpr /*consteval*/ static auto make_type_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::type_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::type_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "csl::typeinfo : unhandled plateform");
#endif
    }

    template <auto value>
    [[nodiscard]] constexpr static auto make_value_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::value_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::value_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "csl::typeinfo : unhandled plateform");
#endif
    }

    /// \brief Widen a narrow name into a CharT array.
    ///        The compiler-provided sources (__PRETTY_FUNCTION__, __FUNCSIG__) are narrow by construction, and have no wide counterpart.
    ///        Per-character cast: valid for the basic execution character set.
    template <typename CharT, std::size_t N>
    [[nodiscard]] constexpr static auto widen(std::string_view value) -> std::array<CharT, N> {
        std::array<CharT, N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result[i] = static_cast<CharT>(static_cast<unsigned char>(value[i]));
        return result;
    }
}

/// \brief constexpr typeinfo that does not relies on __cpp_rtti
///
/// WARNING: Produced outputs ARE NOT portable.
///          Inconsistencies exist across compilers (GCC, Clang, msvc-cl).
///
/// Known limitations :
///
///  [type_name]
///     decl namespace (demo: https://godbolt.org/z/vhr4h4j9Y)
///            GCC:    namespace is part of the type (ex: A::B::C::type_as_ns<>::func<>(int)::my_type)
///            clang:  missing (ex: my_type)
///     type aliases (demo: https://godbolt.org/z/9PEq9zWTn)
///        ex : std::string
///            std::basic_string<char>
///            std::__cxx11::basic_string<char>
///            std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>
///     MSVC: "class "/"struct " prefixes stripped at the top level only
///
///  [value_name]
///     values representation (demo: https://godbolt.org/z/bn9ofo3Pz)
///      ex : int{42}
///          GCC, Clang: "42"sv
///          MSVC:       "0x2a"sv
///      use <charconv> std::to_chars into std::string_view for reliable basic numerical values
namespace csl::typeinfo
{
    template <typename T>
    struct type_name {
        constexpr static auto value = details::make_type_name<T>();
    };
    template <typename T>
    constexpr inline static auto type_name_v = type_name<T>::value;

    /// \brief type_name<T> widened to CharT, e.g. type_name_as_v<int, wchar_t> == L"int".
    template <typename T, typename CharT>
    struct type_name_as {
    private:
        constexpr static auto storage = details::widen<CharT, type_name<T>::value.size()>(type_name<T>::value);
    public:
        constexpr static std::basic_string_view<CharT> value{ storage.data(), storage.size() };
    };
    template <typename T>
    struct type_name_as<T, char> : type_name<T>{};

    template <typename T, typename CharT>
    constexpr inline static auto type_name_as_v = type_name_as<T, CharT>::value;


    template <auto v>
    struct value_name {
        constexpr static auto value = details::make_value_name<v>();
    };
    template <auto v>
    constexpr inline static auto value_name_v = value_name<v>::value;

    /// \brief value_name<v> widened to CharT, e.g. value_name_as_v<42, wchar_t> == L"42".
    template <auto v, typename CharT>
    struct value_name_as {
    private:
        constexpr static auto storage = details::widen<CharT, value_name<v>::value.size()>(value_name<v>::value);
    public:
        constexpr static std::basic_string_view<CharT> value{ storage.data(), storage.size() };
    };
    template <auto v>
    struct value_name_as<v, char> : value_name<v>{};

    template <auto v, typename CharT>
    constexpr inline static auto value_name_as_v = value_name_as<v, CharT>::value;

    // TODO(Guss): hash_code, see #93 https://github.com/GuillaumeDua/CppShelf/issues/93
    //  - update example accordingly
}
