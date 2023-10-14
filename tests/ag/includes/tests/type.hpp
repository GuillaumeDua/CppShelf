#pragma once

namespace test::ag {
    // NOLINTBEGIN
    template <typename T>
    struct type {
        T           v0;
        T &         v1;
        T &&        v2;
        const T     v3;
        const T &   v4;
        const T &&  v5;
    };
    // NOLINTEND
}