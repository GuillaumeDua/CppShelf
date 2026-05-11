#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#include <csl/ag.hpp>

#include <tests/ag/details/mp.hpp>
#include <tests/ag/conversion/to_tuple.hpp>
#include <tests/ag/conversion/to_tuple_view.hpp>
#include <tests/ag/conversion/make.hpp>
#include <tests/ag/size.hpp>
#include <tests/ag/element.hpp>
#include <tests/ag/get.hpp>
#include <tests/ag/custom_tuple_like_interface.hpp>
#include <tests/ag/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif

TEST_CASE("ag::compile_time [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][compile_time]") {
    // compile-time assertions verified via static_assert in included headers above
}

#undef CSL_AG_BITFIELDS_STR
