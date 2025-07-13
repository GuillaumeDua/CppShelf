// Force CSL_MP_TUPLE__IMPLICIT_CONVERSION=ON
#if not defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
# define CSL_MP_TUPLE__IMPLICIT_CONVERSION true
#endif
#include <csl/mp.hpp>
#if not defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
# error "[Test] csl::ag : expect CSL_MP_TUPLE__IMPLICIT_CONVERSION=ON"
#endif

// parts
#include <tests/mp/tuple.hpp>

auto main() -> int {}

