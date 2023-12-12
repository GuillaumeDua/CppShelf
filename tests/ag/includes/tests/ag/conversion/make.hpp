#pragma once

#include <csl/ag.hpp>
#include <cstdint>
#include <array>
#include <vector>
#include "tests/types.hpp"

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
    static_assert(std::same_as<
        longer,
        decltype(csl::ag::make<longer>(std::declval<smaller>()))
    >);
    static_assert(std::same_as<
        longer,
        decltype(std::declval<smaller>() | csl::ag::to<longer>())
    >);
}
namespace test::ag::conversion::make_::complete::non_narrowing_conversion {
    struct int_aggregate { std::int32_t i; };
    struct long_aggregate { std::int64_t l; };
    static_assert(csl::ag::concepts::convertible_to<int_aggregate, long_aggregate>);
    static_assert(std::same_as<
        long_aggregate,
        decltype(csl::ag::make<long_aggregate>(std::declval<int_aggregate>()))
    >);
    static_assert(std::same_as<
        long_aggregate,
        decltype(std::declval<int_aggregate>() | csl::ag::to<long_aggregate>())
    >);
}
namespace test::ag::conversion::make::template_type::ttp {
    struct type { int v0; int v1; };
    using expected = std::vector<int>;

    static_assert(csl::ag::concepts::convertible_to<type, expected>);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<std::vector>(std::declval<type>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(std::declval<type>() | csl::ag::to<std::vector>())
    >);
}
namespace test::ag::conversion::make::template_type::ttps {
    struct type { int v0; int v1; };
    using expected = std::tuple<int, int>;

    static_assert(csl::ag::concepts::convertible_to<type, expected>);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<std::tuple>(std::declval<type>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(std::declval<type>() | csl::ag::to<std::tuple>())
    >);
}
namespace test::ag::conversion::make::template_type::ttp_nttp {
    struct type { int v0; int v1; };
    using expected = std::array<int, 2>;

    static_assert(csl::ag::concepts::convertible_to<type, expected>);
    static_assert(std::same_as<
        expected,
        decltype(csl::ag::make<std::array>(std::declval<type>()))
    >);
    static_assert(std::same_as<
        expected,
        decltype(std::declval<type>() | csl::ag::to<std::array>())
    >);
}
