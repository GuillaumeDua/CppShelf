#pragma once

#if __cplusplus >= 202002L
# include <csl/cxx_20/ensure.hpp>
#elif __cplusplus >= 201703L
# include <csl/cxx_17/ensure.hpp>
#else
# error "csl::ensure: requires at least C++17"
#endif
