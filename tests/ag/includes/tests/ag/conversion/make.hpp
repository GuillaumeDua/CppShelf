#pragma once

#include <csl/ag.hpp>
#include <cstdint>
#include "tests/type.hpp"

namespace test::ag::conversion::make_ {
    using type = test::ag::types::aggregate_all_cvref<int>;
}
namespace test::ag::conversion::make_::complete {
    struct expected{ int v0; int v1; int v2; int v3; int v4; int v5; };
    static_assert(csl::ag::concepts::convertible_to<type, expected>);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<expected>(std::declval<type&>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<expected>(std::declval<type&&>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<expected>(std::declval<const type&>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<expected>(std::declval<const type &&>()))
    >);
}
namespace test::ag::conversion::make_::complete::to_larger {
    struct smaller { int i; };
    struct longer { int i; char c; };
    static_assert(csl::ag::concepts::convertible_to<smaller, longer>);
}
namespace test::ag::conversion::make_::complete::non_narrowing_conversion {
    struct int_aggregate { std::int32_t i; };
    struct long_aggregate { std::int64_t l; };
    static_assert(csl::ag::concepts::convertible_to<int_aggregate, long_aggregate>);
}
