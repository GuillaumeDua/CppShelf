#include <csl/ag.hpp>

auto main() -> int {
    struct A { char a, b, c, d, e, f, g, h; };
    static_assert(sizeof(A) == 8);
    static_assert(csl::ag::size_v<A> == 8);

    struct B { int a, b; };
    static_assert(sizeof(B) == 8);
    static_assert(csl::ag::size_v<B> == 2);

    struct alignas(32) C { char c; };
    static_assert(sizeof(C) == 32);
    static_assert(csl::ag::size_v<C> == 1);
}
