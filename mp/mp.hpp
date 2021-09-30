#pragma once

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)
#define static_dependent_error(message) static_assert([](){ return false; }(), message)

// cpp shelf library : metaprogramming
namespace csl::mp {
    
}

#undef fwd
#undef static_dependent_error