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
