#pragma once

#include <csl/ag.hpp>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace test::ag {
    template <typename T>
    struct S {
        T v;
        const T c_v;
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    };
    using type = S<int>;
}

namespace test::ag::size_ {
    static_assert(csl::ag::size_v<type> == 6);
    // todo : check not requires csl::ag::size<type /*cvref qualifier*/>
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
namespace test::ag::get_ {
    template <typename T>
    consteval void check_type_impl() {
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            static_assert((std::same_as<
                decltype(std::get<indexes>(std::declval<T>())),
                csl::ag::view_element_t<indexes, T>
            > and ...));
        }(std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<T>>>{});
    }
    consteval void check_type() {
        check_type_impl<type&>();
        check_type_impl<type&&>();
        check_type_impl<const type&>();
        check_type_impl<const type&&>();
    }
}