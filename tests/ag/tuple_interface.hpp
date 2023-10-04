#pragma once

#include <csl/ag.hpp>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace test::ag {
    template <typename T>
    struct S {
    // NOLINTBEGIN(*-avoid-const-or-ref-data-members)
        T v;
        const T c_v;
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    // NOLINTEND(*-avoid-const-or-ref-data-members)
        using as_tuple_t = std::tuple<T, const T, T&, const T &, T&&, const T&&>;
    };
    using type = S<int>;
    using type_as_tuple = type::as_tuple_t;
}

namespace test::ag::size_ {
    constexpr auto expected_size_v = 6;
    static_assert(csl::ag::size_v<type> == expected_size_v);
}
namespace test::ag::element_ {

    template <typename ...>
    struct pack{};

    template <csl::ag::concepts::aggregate T>
    using fields_t = decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        return pack<
            csl::ag::element_t<indexes, T>...
        >{};
    }(std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<T>>>{}));

    static_assert(std::same_as<
        fields_t<type>,
        pack<int, const int, int&, const int&, int&&, const int&&>
    >);
}
namespace test::as::std_tuple_element {
    consteval void check(){
        // std::tuple_element<std::size_t, T>
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            static_assert((std::same_as<
                std::tuple_element_t<indexes, std::remove_cvref_t<test::ag::type>>,
                std::tuple_element_t<indexes, typename test::ag::type::as_tuple_t>
            > and ...));
        }(std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<test::ag::type>>>{});
    }
}
namespace test::ag::get_ {
    template <typename T>
    consteval void check_type_impl() {

        // std::get<std::size_t>
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            static_assert((std::same_as<
                decltype(std::get<indexes>(std::declval<T>())),
                csl::ag::view_element_t<indexes, T>
            > and ...));
        }(std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<T>>>{});

        // std::get<T>
    }
    consteval void check_type() {
        check_type_impl<type&>();
        check_type_impl<type&&>();
        check_type_impl<const type&>();
        check_type_impl<const type&&>();
    }
}
