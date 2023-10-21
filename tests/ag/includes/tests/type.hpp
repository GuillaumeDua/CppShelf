#pragma once

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
    };

    using aggregate_1 = struct { int i; };
    using aggregate_2 = struct { int i; char c; };

    using aggregate_ref_1 = struct { int & i; };
    using aggregate_ref_2 = struct { int & i; char && c; };
    using aggregate_ref_3 = struct { int & i; char && c; char & cc; };
}