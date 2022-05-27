#pragma once

#include <type_traits>
#include <utility>
#include <tuple>
#include <functional>
#include <array>
#include <string_view>
#include <iostream>
#include <sstream>
#include <concepts>
#include <bit>

// ---

namespace gcl::cx
{   // constexpr typeinfo that does not relies on __cpp_rtti
    template <typename T>
    static constexpr /*consteval*/ auto type_name(/*no parameters allowed*/) -> std::string_view
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
    static constexpr auto type_name(/*no parameters allowed*/) -> std::string_view
    {
        return type_name<decltype(value)>();
    }
}

namespace gcl::cx
{
    using hash_type = uint32_t;
    struct crc_32
    {
        constexpr static hash_type hash(std::string_view str)
        {
            hash_type crc = 0xffffffff;
            for (auto c : str)
                crc = (crc >> 8) ^ crc_table[(crc ^ c) & 0xff];
            return crc ^ 0xffffffff;
        }

      private:
        static constexpr hash_type crc_table[256] = {
            0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
            0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
            0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
            0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
            0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
            0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
            0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
            0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
            0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
            0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
            0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
            0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
            0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
            0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
            0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
            0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
            0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
            0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
            0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
            0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
            0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
            0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
            0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
            0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
            0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
            0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
            0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
            0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
            0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};
    };
}

template <typename T, typename hash_type = gcl::cx::crc_32>
constexpr auto type_name_hash_v = hash_type::hash(gcl::cx::type_name_v<T>);

// ---

// TODO : versioning, compatibility
// TODO : cross-compiler
// TODO : best default initialization : is_aggregate ? aggregate_initialization : binary
// TODO : Different description for write and read
//        read  : std::invocable<accessor_t, T> and std::assignable_from<std::invoke_result_t<accessor_t, T>, std::invoke_result_t<accessor_t, T>>
//        write : std::invocable<accessor_t, T> and invoke_result not void

// ---

namespace csl::srl {
    template <typename T>
    struct description; // customization point (opt-in)
}
namespace csl::srl::descriptions {
// allowed types for `typename description<T>::type`
    template <auto ...>
    struct constructor_initialization{};
    template <auto ...>
    struct aggregate_initialization{};
    template <auto ...>
    struct defered_initialization{};
}
namespace csl::srl::mp {

    // is_specific description type
    template <typename T>
    struct is_constructor_initialization : std::false_type{};
    template <auto ... nttps>
    struct is_constructor_initialization<descriptions::constructor_initialization<nttps...>> : std::true_type{};
    template <typename T>
    struct is_aggregate_initialization : std::false_type{};
    template <auto ... nttps>
    struct is_aggregate_initialization<descriptions::aggregate_initialization<nttps...>> : std::true_type{};
    template <typename T>
    struct is_defered_initialization : std::false_type{};
    template <auto ... nttps>
    struct is_defered_initialization<descriptions::defered_initialization<nttps...>> : std::true_type{};

    template <typename T>
    constexpr bool is_constructor_initialization_v = is_constructor_initialization<T>::value;
    template <typename T>
    constexpr bool is_aggregate_initialization_v = is_aggregate_initialization<T>::value;
    template <typename T>
    constexpr bool is_defered_initialization_v = is_defered_initialization<T>::value;

    // is_allowed_description_type
    template <typename T>
    struct is_allowed_description_type : std::bool_constant<
        is_constructor_initialization_v<T> or
        is_aggregate_initialization_v<T> or
        is_defered_initialization_v<T>
    >{};
    template <typename T>
    constexpr bool is_allowed_description_type_v = is_allowed_description_type<T>::value;
}

namespace csl::srl::details::concepts {
    template <typename T, typename ... args_ts>
    concept AggregateInitializable = requires { T{ std::declval<args_ts>()... }; };

    template <typename T, typename target>
    concept AccessorOf = 
        std::invocable<T, target> and
        not std::same_as<void, std::invoke_result_t<T, target>>
    ;

    template <typename T>
    concept Unqualified =
        // [csl::srl] cvref-qualified
        not std::is_reference_v<T> and
        not std::is_const_v<T> and
        not std::is_volatile_v<T> and
        // [csl::srl] not raw-pointers
        not std::is_pointer_v<T> and
        // [csl::srl] not unions
        not std::is_union_v<T> and
        // [csl::srl] no empty type (?)
        not std::is_empty_v<T>
    ;   // what about is_polymorphic_v ?

    template <typename T>
    concept HasDescription = // might still not be a valid/allowed one
        details::concepts::Unqualified<T> and
        // [csl::srl] not defined description type
        requires { typename csl::srl::description<T>::type; }
    ;

    template <typename T>
    concept DescribedConstructible =
        HasDescription<T> and
        mp::is_constructor_initialization_v<typename csl::srl::description<T>::type> and
        []<auto ... accessors>(descriptions::constructor_initialization<accessors...>){
            // [csl::srl] Warning : Clang does not implement P0960 yet (Parenthesized initialization of aggregate)
            return std::constructible_from<
                T,
                std::invoke_result_t<decltype(accessors), T>...
            >;
        }(typename csl::srl::description<T>::type{})
    ;
    template <typename T>
    concept DescribedAggregate =
        HasDescription<T> and
        mp::is_aggregate_initialization_v<typename csl::srl::description<T>::type> and
        []<auto ... accessors>(descriptions::aggregate_initialization<accessors...>){
            return details::concepts::AggregateInitializable<
                T,
                std::invoke_result_t<decltype(accessors), T>...
            >;
        }(typename csl::srl::description<T>::type{})
    ;
    template <typename T>
    concept DescribedDefered =
        HasDescription<T> and
        mp::is_defered_initialization_v<typename csl::srl::description<T>::type> and
        std::is_default_constructible_v<T> and
        []<auto ... accessors>(descriptions::defered_initialization<accessors...>){
            return (... and std::is_assignable_v<
                std::invoke_result_t<decltype(accessors), T&>,
                std::invoke_result_t<decltype(accessors), T>
            >);
        }(typename csl::srl::description<T>::type{})
    ;
}

namespace csl::srl::concepts {

    template <typename T>
    concept BinaryFormattable =
        details::concepts::Unqualified<T> and
        std::is_standard_layout_v<T>
    ;

    template <typename T>
    concept Described =
        (
            details::concepts::DescribedAggregate<T> or
            details::concepts::DescribedConstructible<T> or
            details::concepts::DescribedDefered<T>
        ) and
        // [csl::srl] not allowed description type
        mp::is_allowed_description_type_v<typename csl::srl::description<T>::type> and
        // GCC 12.1 : ICE, Clang : undefined `typename csl::srl::description<T>::type`
        // [csl::srl] accessor is not invocable with T
        []<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
            return ((std::invocable<decltype(accessors), T> or ...));
        }(typename csl::srl::description<T>::type{}) and
        // [csl::srl] likely infinite-recursivity detected
        []<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
            return not ((std::same_as<
                std::remove_cvref_t<std::invoke_result_t<decltype(accessors), T>>,
                T
            > or ...));
        }(typename csl::srl::description<T>::type{})
    ;

    template <typename T>
    concept Formattable = Described<T> or BinaryFormattable<T>;

    // TODO : input, output (writable/readable)
}

namespace csl::srl {

    // internal API
    template <template <typename> typename formatter_t>
    void write(auto & os, auto && data) {
        using type = std::remove_cvref_t<decltype(data)>;
        using formatter_type = formatter_t<type>;
        formatter_type::write(os, std::forward<decltype(data)>(data));
    }
    template <template <typename> typename formatter_t, details::concepts::Unqualified T>
    T read(auto & is) {
        using formatter_type = formatter_t<T>;
        return formatter_type::read(is);
    }
    template <template <typename> typename formatter_t>
    void read_to(auto & is, auto && data) {
        using type = std::remove_cvref_t<decltype(data)>;
        using formatter_type = formatter_t<type>;
        formatter_type::read_to(is, std::forward<decltype(data)>(data));
    }

    // formatter : binary (fallback)
    template <concepts::BinaryFormattable T>
    struct binary_formatter {

        using type = std::remove_cvref_t<T>;

        template <typename ostream_type>
        static decltype(auto) write(ostream_type & os, auto && data)
        {
            static_assert(std::convertible_to<T, std::remove_cvref_t<decltype(data)>>);
            os.write(std::bit_cast<const typename ostream_type::char_type *>(std::addressof(data)), sizeof(type));
        }
        template <typename istream_type>
        static decltype(auto) read_to(istream_type & is, auto && data)
        {
            static_assert(std::convertible_to<T, std::remove_cvref_t<decltype(data)>>);
            is.read(std::bit_cast<typename istream_type::char_type *>(std::addressof(data)), sizeof(type));
        }
        template <typename istream_type>
        static auto read(istream_type & is) -> type
        {
            using type = std::remove_cvref_t<T>;
            type data;
            is.read(std::bit_cast<typename istream_type::char_type *>(std::addressof(data)), sizeof(type));
            return data;
        }
    };

    template <typename T>
    struct formatter;

    template <concepts::BinaryFormattable T>
    requires (not concepts::Described<T>)
    struct formatter<T> : binary_formatter<T>{}; // fallback, customization point (opt-out)

    template <concepts::Described T>
    struct formatter<T> {

        using type = T;

        static void write(auto & os, auto && data)
        {
            static_assert(std::convertible_to<T, std::remove_cvref_t<decltype(data)>>);

            [&]<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
                ((csl::srl::write<formatter>(
                    os,
                    std::invoke(
                        accessors,
                        std::forward<decltype(data)>(data)
                    )
                )), ...);
            }(typename csl::srl::description<type>::type{});
        }
        static void read_to(auto & is, auto && data)
        {
            static_assert(std::convertible_to<T, std::remove_cvref_t<decltype(data)>>);

            [&]<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
                
                ((csl::srl::read_to<formatter>(
                    is,
                    std::invoke(
                        accessors,
                        std::forward<decltype(data)>(data)
                    )
                )), ...);
            }(typename csl::srl::description<type>::type{});
        }

        // dispatch each read value to aggregate|constructor|defered
        // TODO : switch according to concept
        // TODO : proposer recursivity (not necessarely binary)
        static auto read(auto && is) -> type
        requires details::concepts::DescribedAggregate<T>
        {
            return [&]<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
                return type{
                    csl::srl::read<
                        csl::srl::binary_formatter,
                        std::remove_cvref_t<
                            std::invoke_result_t<
                                decltype(accessors),
                                T
                            >
                        >
                    >(is)...
                };
            }(typename csl::srl::description<T>::type{});
        }
        static auto read(auto && is) -> type
        requires details::concepts::DescribedConstructible<T>
        {
            return [&]<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
                return type(
                    csl::srl::read<
                        csl::srl::binary_formatter,
                        std::remove_cvref_t<
                            std::invoke_result_t<
                                decltype(accessors),
                                T
                            >
                        >
                    >(is)...
                );
            }(typename csl::srl::description<T>::type{});
        }
        static auto read(auto && is) -> type
        requires details::concepts::DescribedDefered<T>
        {
            return [&]<template <auto...> typename description_t, auto ... accessors>(description_t<accessors...>){
                type value;
                ((
                    std::invoke(accessors, value) = csl::srl::read<
                        csl::srl::binary_formatter,
                        std::remove_cvref_t<
                            std::invoke_result_t<
                                decltype(accessors),
                                T
                            >
                        >
                    >(is)
                ), ...);
                return value;
            }(typename csl::srl::description<T>::type{});
        }
    };

    // TODO : raw-pointers formatter

    // TODO : STL formatter (in separate file to include, so users can opt-in ?)
    // formatter<std::string>
    // formatter<std::tuple>
    // formatter<std::(const_)reference_wrapper>
    // formatter<std::(unique/shared)_ptr>
}

// ---
// public API :

namespace csl::srl {
    void write(auto & os, auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        using formatter_t = typename csl::srl::formatter<type>;
        formatter_t::template write(os, std::forward<decltype(value)>(value));
    }
    void read_to(auto & is, auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        using formatter_t = typename csl::srl::formatter<type>;
        formatter_t::template read_to(is, std::forward<decltype(value)>(value));
    }
    template <csl::srl::concepts::Described T>
    auto read(auto & is) -> T {
        using formatter_t = typename csl::srl::formatter<T>;
        return formatter_t::template read(is);
    }
    template <csl::srl::concepts::BinaryFormattable T>
    requires (not csl::srl::concepts::Described<T>)
    auto read(auto & is) -> T {
        using formatter_t = csl::srl::binary_formatter<T>;
        return formatter_t::template read(is);
    }
}

// ---
// Automated serialization

namespace gcl::cx
{
    template <typename Key, typename Value, std::size_t N>
    struct unordered_map {

        using key_type = Key;
        using mapped_type = Value;
        using type = unordered_map<Key, Value, N>;
        using value_type = std::pair<const key_type, mapped_type>;

        constexpr static auto size = N;

        consteval unordered_map(auto && ... args)
            : storage{ value_type{ get<0>(args), get<1>(args) }... }
        {}

        [[nodiscard]] consteval auto & at(const key_type & arg) const
        {
            auto it = std::find_if(
                std::cbegin(storage), std::cend(storage),
                [&arg](const auto& element) constexpr { return element.first == arg; }
            );
            if (it == std::cend(storage))
                throw std::out_of_range{ "gcl::cx::unordered_map::at" };
            return it->second;
        }

        constexpr decltype(auto) begin()        { return std::begin(storage); }
        constexpr decltype(auto) end()          { return std::end(storage); }
        constexpr decltype(auto) begin()  const { return std::begin(storage); }
        constexpr decltype(auto) end()    const { return std::end(storage); }
        constexpr decltype(auto) cbegin() const { return std::cbegin(storage); }
        constexpr decltype(auto) cend()   const { return std::cend(storage); }

    private:
        using storage_type = std::array<value_type, N>;
        const storage_type storage;
    };

    unordered_map(auto &&... args) -> unordered_map<
        std::common_type_t<decltype(get<0>(args))...>,
        std::common_type_t<decltype(get<1>(args))...>,
        sizeof...(args)
    >;
}