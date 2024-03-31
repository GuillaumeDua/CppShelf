// Force CSL_AG_ENABLE_BITFIELDS_SUPPORT=ON
#if not defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# define CSL_AG_ENABLE_BITFIELDS_SUPPORT true // force bitfield support. Prefer using CMake cache instead CSL_AG_ENABLE_BITFIELDS_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG_ENABLE_BITFIELDS_SUPPORT=OFF"
#endif

#include "includes/tests/ag/size.hpp"

auto main() -> int {}
