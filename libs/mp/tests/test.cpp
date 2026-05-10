#if not defined(FORCE_CSL_MP_TUPLE__IMPLICIT_CONVERSION)
#    error "FORCE_CSL_MP_TUPLE__IMPLICIT_CONVERSION is not set"
#endif
#if defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
#   undef CSL_MP_TUPLE__IMPLICIT_CONVERSION
#endif
#define CSL_MP_TUPLE__IMPLICIT_CONVERSION FORCE_CSL_MP_TUPLE__IMPLICIT_CONVERSION

#include <csl/mp.hpp>

// parts
#include <tests/mp/primitive.hpp>
#include <tests/mp/sequence.hpp>
#include <tests/mp/tuple.hpp>
#include <tests/mp/tuple_algorithm.hpp>
#include <tests/mp/functions.hpp>
// #include <tests/mp/flat.hpp>

#include <catch2/catch_test_macros.hpp>

#if CSL_MP_TUPLE__IMPLICIT_CONVERSION == 0
#  define CSL_MP_TUPLE__IMPLICIT_CONVERSION_STR "NONE"
#elif CSL_MP_TUPLE__IMPLICIT_CONVERSION == 1
#  define CSL_MP_TUPLE__IMPLICIT_CONVERSION_STR "SAFE"
#elif CSL_MP_TUPLE__IMPLICIT_CONVERSION == 2
#  define CSL_MP_TUPLE__IMPLICIT_CONVERSION_STR "UNSAFE"
#else
# error "Unexpected CSL_MP_TUPLE__IMPLICIT_CONVERSION value"
#endif

TEST_CASE("mp::compile_time [IMPLICIT_CONVERSION=" CSL_MP_TUPLE__IMPLICIT_CONVERSION_STR "]", "[mp][compile_time]") {
    // compile-time assertions verified via static_assert in included headers above
}
