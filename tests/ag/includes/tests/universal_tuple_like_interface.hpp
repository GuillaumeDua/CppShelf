#pragma once

#include <csl/ag.hpp>

namespace test::universal_::types {
    struct aggregate { int i; char c; };

    using all_t = std::tuple<
        aggregate
    >;
}
namespace test::universal_ {

    void get(){
        
    }

    // tuple_size, _v

    // tuple_element, _t
}
