// Force CSL_AG_ENABLE_BITFIELDS_SUPPORT=OFF
#if defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# undef CSL_AG_ENABLE_BITFIELDS_SUPPORT
#endif
#include <csl/ag.hpp>
#if defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG_ENABLE_BITFIELDS_SUPPORT=OFF"
#endif

#include "includes/tests/ag/details/mp.hpp"
#include "includes/tests/ag/conversion/to_tuple.hpp"
#include "includes/tests/ag/conversion/to_tuple_view.hpp"
#include "includes/tests/ag/size.hpp"
#include "includes/tests/ag/element.hpp"
#include "includes/tests/ag/get.hpp"

auto main() -> int {}
