// Force CSL_AG__ENABLE_BITFIELDS_SUPPORT=OFF
#if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
# undef CSL_AG__ENABLE_BITFIELDS_SUPPORT
#endif
#include <csl/ag.hpp>
#if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_BITFIELDS_SUPPORT=OFF"
#endif

#include "tests/ag/details/mp.hpp"
#include "tests/ag/conversion/to_tuple.hpp"
#include "tests/ag/conversion/to_tuple_view.hpp"
#include "tests/ag/conversion/make.hpp"
#include "tests/ag/size.hpp"
#include "tests/ag/element.hpp"
#include "tests/ag/get.hpp"
#include "tests/ag/custom_tuple_like_interface.hpp"

auto main() -> int {}
