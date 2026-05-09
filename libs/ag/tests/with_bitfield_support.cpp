// Force CSL_AG__ENABLE_BITFIELDS_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
# define CSL_AG__ENABLE_BITFIELDS_SUPPORT true // force bitfield support. Prefer using CMake cache instead CSL_AG__ENABLE_BITFIELDS_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_BITFIELDS_SUPPORT=ON"
#endif

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

TEST_CASE("ag::compile_time (bitfield support)", "[ag][compile_time]") {
    // compile-time assertions verified via static_assert in included headers above
}
