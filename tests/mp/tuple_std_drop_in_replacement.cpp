// Force CSL_MP_TUPLE__IMPLICIT_CONVERSION=ON
#if not defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
# define CSL_MP_TUPLE__IMPLICIT_CONVERSION UNSAFE
#endif
#include <csl/mp.hpp>
#if not defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
# error "[Test] csl::ag : expect CSL_MP_TUPLE__IMPLICIT_CONVERSION=ON"
#endif

// parts
#include <tests/mp/tuple.hpp>

auto main() -> int {}

// TODO(Guillaume) just replace by other test targets