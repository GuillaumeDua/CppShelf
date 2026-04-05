#pragma once

#include <csl/mp.hpp>

namespace test::type_traits::type_identity {
    static_assert(std::same_as<int, csl::mp::type_identity_t<int>>);
}
namespace test::type_traits::unwrap_reference {
    static_assert(std::same_as<int, csl::mp::unwrap_reference_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_reference_t<std::reference_wrapper<int>>>);
    static_assert(not std::same_as<int, csl::mp::unwrap_reference_t<int&>>);

    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_ref_decay_t<std::reference_wrapper<int>>>);
    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int&>>);
}
namespace test::concepts::fwd_ref {
    static_assert(csl::mp::concepts::fwd_ref<int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<int&&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&&, int>);

    static_assert(not csl::mp::concepts::fwd_ref<int&, int&>);
}
#include <string>
namespace test::concepts::instance {
    static_assert(csl::mp::concepts::instance<std::vector<int>, std::vector>);
    static_assert(csl::mp::concepts::instance<std::string, std::basic_string>);
    static_assert(csl::mp::concepts::instance<const std::string &, std::basic_string>);
}

namespace test::primitives::value_type {
    static_assert(std::same_as<char, csl::mp::value_type_t<std::string>>);
    static_assert(std::same_as<char, csl::mp::value_type_t<std::array<char, 1>>>);
    static_assert(std::same_as<char, csl::mp::value_type_t<char[1]>>); // NOLINT(*-c-arrays)
    static_assert(std::same_as<bool, csl::mp::value_type_t<std::is_same<int, char>>>);
}
namespace test::primitives::bind_front {
    using same_as_int = csl::mp::bind_front<std::is_same, int>;
    static_assert(same_as_int::value<int>);

    using tuple_starting_with_int = csl::mp::bind_front<csl::mp::tuple, int>;
    static_assert(std::same_as<
        tuple_starting_with_int::type<char>,
        csl::mp::tuple<int, char>
    >);
}
namespace test::primitives::bind_back {
    using same_as_int = csl::mp::bind_back<std::is_same, int>;
    static_assert(same_as_int::value<int>);

    using tuple_ending_with_int = csl::mp::bind_back<csl::mp::tuple, int>;
    static_assert(std::same_as<
        tuple_ending_with_int::type<char>,
        csl::mp::tuple<char, int>
    >);
}

namespace test::primitives::index_map {

    struct A{};
    struct B{};
    struct C{};
    struct D{};
    struct E{};
    struct F{};

    constexpr static auto expected = csl::mp::index_map_t<6>{
        .tuples = { 0, 0, 1, 2, 2, 3 },
        .elements = { 0, 1, 0, 0, 1, 0 },
    };
    using expected_t = std::remove_cvref_t<decltype(expected)>;

    static_assert(std::is_same_v<
        expected_t,
        csl::mp::make_index_map_t<
            csl::mp::tuple<A, B>,
            std::array<C, 1>,
            std::pair<D, E>,
            std::tuple<F>
        >
    >);
    static_assert(expected == csl::mp::index_map_v<
        csl::mp::tuple<A, B>,
        std::array<C, 1>,
        std::pair<D, E>,
        std::tuple<F>
    >);

    // Check multiples empty tuplelikes
    static_assert(
        csl::mp::index_map_t<1>{
            .tuples = { 2 },
            .elements = { 0 }
        }
    ==  csl::mp::index_map_v<
            csl::mp::tuple<>,
            std::tuple<>,
            std::array<A, 1>, // non-empty
            csl::mp::tuple<>,
            csl::mp::tuple<>
        >
    );
}
