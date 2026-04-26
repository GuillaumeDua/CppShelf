#pragma once

#include <csl/mp.hpp>

namespace test::mp {

    template <typename ...>
    struct pack{};
    struct A{}; struct B{}; struct C{};

    using namespace csl::mp;
}

// flatten_once
namespace test::mp::flatten_once_ {
    static_assert(std::is_same_v<int, flatten_once_t<int>>);

    static_assert(std::is_same_v<flatten_once_t<pack<A,B,C>>, pack<A,B,C>>);
    using t0 = pack<pack<pack<A>>, pack<B>, C>;

    using t1 = flatten_once_t<t0>;  static_assert(std::is_same_v<t1, pack<pack<A>, B, C>>);
    using t2 = flatten_once_t<t1>;  static_assert(std::is_same_v<t2, pack<A, B, C>>);
    using t3 = flatten_once_t<t2>;  static_assert(std::is_same_v<t3, pack<A, B, C>>);
}
// flatten
namespace test::mp::flatten_ {
    using type = pack<pack<pack<A>>, pack<B>, C, pack<pack<A>>, pack<B, C>, A, B, C>;
    static_assert(std::is_same_v<flatten_t<type>, pack<A,B,C,A,B,C,A,B,C>>);
}
// flat_cat_
namespace test::mp::flat_cat_ {
    static_assert(std::is_same_v<
        flat_cat_t<pack<A>, pack<B, C>>,
        pack<A,B,C>
    >);
    static_assert(std::is_same_v<
        flat_cat_t<pack<A>, B, C>,
        pack<A,B,C>
    >);
}