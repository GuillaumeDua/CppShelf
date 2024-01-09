#pragma once

#include <csl/ag.hpp>

namespace test::ag::size_ {
    template <typename T>
    concept has_consistent_size = csl::ag::concepts::aggregate<T>
    and csl::ag::size_v<T> == csl::ag::tuple_size_v<T>
    and csl::ag::size_v<T> == std::tuple_size_v<csl::ag::to_tuple_t<T>>
    and csl::ag::size_v<T> == std::tuple_size_v<csl::ag::view_t<T&>>
    and csl::ag::size_v<T> == std::tuple_size_v<csl::ag::view_t<T&&>>
    and csl::ag::size_v<T> == std::tuple_size_v<csl::ag::view_t<const T&>>
    and csl::ag::size_v<T> == std::tuple_size_v<csl::ag::view_t<const T&&>>
    ;
}

#if not defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
#pragma message("[csl::ag] test size with CSL_AG_ENABLE_BITFIELDS_SUPPORT=OFF")
namespace test::ag::size_::trivial_cases {
    using expect_1 = struct { int i; };
    using expect_2 = struct { int i; char c; };

    static_assert(has_consistent_size<expect_1>);
    static_assert(has_consistent_size<expect_2>);

    static_assert(csl::ag::size_v<expect_1> == 1);
    static_assert(csl::ag::size_v<expect_2> == 2);
}
namespace test::ag::size_::non_trivial_cases {
    // NOLINTBEGIN
    using expect_1 = struct { int & i; };
    using expect_2 = struct { int & i; char && c; };
    using expect_3 = struct { int & i; char && c; char & cc; };
    // NOLINTEND

    static_assert(has_consistent_size<expect_1>);
    static_assert(has_consistent_size<expect_2>);
    static_assert(has_consistent_size<expect_3>);

    static_assert(csl::ag::size_v<expect_1> == 1);
    static_assert(csl::ag::size_v<expect_2> == 2);
    static_assert(csl::ag::size_v<expect_3> == 3);
}
#else
namespace test::ag::size_::trivial_cases {
#pragma message("[csl::ag] test size with CSL_AG_ENABLE_BITFIELDS_SUPPORT=ON")
    template <typename T, size_t expected_fields_count>
    consteval auto is_simple_construction_valid() -> bool {
        return
            []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool {
                return ((csl::ag::concepts::aggregate_constructible_from_n_values<T, indexes + 1> and ...));
            }(std::make_index_sequence<expected_fields_count>{})
        and csl::ag::size_v<T> == expected_fields_count
        ;
    }

    template <std::size_t /*field_count*/>
    struct type;
    template <> struct type<1> { int b0 : 1; };
    template <> struct type<2> { int b0 : 1, b1 : 1; };
    template <> struct type<3> { int b0 : 1, b1 : 1, b2 : 1; };
    template <> struct type<4> { int b0 : 1, b1 : 1, b2 : 1, b3 : 1; };
    template <> struct type<5> { int b0 : 1, b1 : 1, b2 : 1, b3 : 1, b4 : 1; };

    template <std::size_t field_count>
    concept is_type_valid = 
        sizeof(type<field_count>) == sizeof(int)
    and csl::ag::size_v<type<field_count>> == field_count
    and has_consistent_size<type<field_count>>
    and is_simple_construction_valid<type<field_count>, field_count>()    
    ;

    [[maybe_unused]] constexpr auto test = []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool {
        static_assert((is_type_valid<indexes + 1> and ...));
        return {};
    }(std::make_index_sequence<5>{});
}
namespace test::ag::size_::non_trivial_cases {
    struct s_5_fields {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
        char && c; // prevent initialization with [N != 5] values
    };
    static_assert(csl::ag::size_v<s_5_fields> == 5);
    static_assert(has_consistent_size<s_5_fields>);
    struct s_5_fields_with_padding {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
        char : 0;
        char && c; // prevent initialization with [N != 5] values
    };
    static_assert(csl::ag::size_v<s_5_fields_with_padding> == 5);
    static_assert(has_consistent_size<s_5_fields_with_padding>);
}
#endif