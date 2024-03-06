#pragma once

#include "tests/types.hpp"
#include <csl/ag.hpp>
#include <memory>
#include <type_traits>

namespace test::ag::custom_tuple_like_interface {
    namespace userland = test::ag::types::custom_get;
}

namespace test::ag::custom_tuple_like_interface::details {

    template <typename T, char expected>
    constexpr decltype(auto) ensure_unqualified_get() noexcept {
        using csl::ag::get;
        using std::get;
        static_assert(requires{ get<0>(std::declval<T>()); });
        static_assert(requires{ get<char>(std::declval<T>()); });
        static_assert(get<0>(T{}) == expected);
        static_assert(get<char>(T{}) == expected);
    }
}
namespace test::ag::custom_tuple_like_interface {

    template <typename ... Ts>
    constexpr static void ensure_unqualified_get(){
        ((details::ensure_unqualified_get<typename Ts::input, Ts::expected>()), ...);
    }

    template <typename T, char value>
    struct test_case {
        using input = T;
        constexpr static inline auto expected = value;
    };

    constexpr static void test(){
        using namespace userland;
        ensure_unqualified_get<
            test_case<A, 'A'>,
            test_case<B, 'B'>,
            test_case<C, 'C'>,
            test_case<D, 'D'>,
            test_case<E, 'E'>,
            test_case<F, 'F'>
        >();
    }
}
