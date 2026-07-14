#pragma once

// NOTE: Detectable by other csl libraries/headers (via #if defined(...)), even when not reachable through __has_include (e.g. Compiler Explorer raw-URL includes).
#define CSL_ENSURE__INCLUDED

#if __cplusplus >= 202002L
# include <csl/cxx_20/ensure.hpp>
#elif __cplusplus >= 201703L
# include <csl/cxx_17/ensure.hpp>
#else
# error "csl::ensure: requires at least C++17"
#endif

// Formatting backends are opt-in feature headers, include them consistently program-wide:
//
//  <csl/ensure/formatting/backend/std_format.hpp>  std::formatter support (requires C++20)
//  <csl/ensure/formatting/backend/fmt.hpp>         fmt::formatter support (fmtlib is provided by the consumer)
//  <csl/ensure/formatting/backend/ostream.hpp>     operator<<(std::ostream &, ...) support
