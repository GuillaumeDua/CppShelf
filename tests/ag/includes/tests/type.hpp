#pragma once

#include <tuple>

namespace test::ag::types {
    template <typename T>
    struct aggregate_all_cvref {
        // NOLINTBEGIN(*-avoid-const-or-ref-data-members)
        T           v0;
        T &         v1;
        T &&        v2;
        const T     v3;
        const T &   v4;
        const T &&  v5;
        // NOLINTEND(*-avoid-const-or-ref-data-members)
        using expected_to_tuple_t = std::tuple<int, int &, int &&, const int, const int &, const int &&>;
    };
    template <typename T>
    struct aggregate_all_cvref_with_std_tuple_interface : aggregate_all_cvref<T>{
        struct csl_optins {
            struct ag {
                using std_tuple_interface = void;
            };
        };
    };

    using aggregate_1 = struct { int i; };
    using aggregate_2 = struct { int i; char c; };

    using aggregate_ref_1 = struct { int & i; };
    using aggregate_ref_2 = struct { int & i; char && c; };
    using aggregate_ref_3 = struct { int & i; char && c; char & cc; };
}
