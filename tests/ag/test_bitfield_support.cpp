#define CSL_AG_ENABLE_BITFIELDS_SUPPORT true // force bitfield support. Prefer using CMake cache instead CSL_AG_ENABLE_BITFIELDS_SUPPORT=ON
#include <csl/ag.hpp>

#if not defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG_ENABLE_BITFIELDS_SUPPORT=ON"
#endif

namespace test::fields_count::trivial_cases {

    template <typename T, size_t expected_fields_count>
    consteval auto is_simple_construction_valid() -> bool {
        return
            []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool {
                return ((csl::ag::concepts::aggregate_constructible_from_n_values<T, indexes + 1> and ...));
            }(std::make_index_sequence<expected_fields_count>{}) and
            csl::ag::details::fields_count<T> == expected_fields_count
        ;
    }

    struct s_1_fields {
        int b0 : 1;
    };
    static_assert(sizeof(s_1_fields) == sizeof(int));
    static_assert(is_simple_construction_valid<s_1_fields, 1>());
    struct s_2_fields {
        int b0 : 1, b1 : 1;
    };
    static_assert(sizeof(s_2_fields) == sizeof(int));
    static_assert(is_simple_construction_valid<s_2_fields, 2>());
    struct s_3_fields {
        int b0 : 1, b1 : 1, b2 : 1;
    };
    static_assert(sizeof(s_3_fields) == sizeof(int));
    static_assert(is_simple_construction_valid<s_3_fields, 3>());
    struct s_4_fields {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
    };
    static_assert(sizeof(s_4_fields) == sizeof(int));
    static_assert(is_simple_construction_valid<s_4_fields, 4>());
    struct s_5_fields {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1, b4 : 1;
    };
    static_assert(sizeof(s_5_fields) == sizeof(int));
    static_assert(is_simple_construction_valid<s_5_fields, 5>());
}
namespace test::fields_count::non_trivial_cases {
    struct s_5_fields {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
        char && c; // prevent initialization with [N != 5] values
    };
    static_assert(csl::ag::details::fields_count<s_5_fields> == 5);
    struct s_5_fields_with_padding {
        int b0 : 1, b1 : 1, b2 : 1, b3 : 1;
        char : 0;
        char && c;
    };
    static_assert(csl::ag::details::fields_count<s_5_fields_with_padding> == 5);
}

auto main() -> int {
}