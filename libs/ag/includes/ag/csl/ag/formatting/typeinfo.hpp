#pragma once

/// @file
/// @brief csl::typeinfo support for the @c typenamed formatting option (opt-in feature header).
///
/// cpp shelf library : aggregates utility - formatting.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @details
/// Backs the @c typenamed formatting option (@c csl::ag::io::type_name customization trait) with csl::typeinfo:
/// compile-time, demangled type names, instead of the @c <typeindex> runtime fallback.
///
/// Orthogonal to csl::ag formatting backends: enhances whichever formatting feature headers are included, if any.
///
/// @note Like every feature header, include it consistently program-wide.

#if not defined(CSL_AG__INCLUDED)
#   include <csl/ag.hpp>
#endif

#if not defined(CSL_TYPEINFO__INCLUDED) and __has_include(<csl/typeinfo.hpp>)
#   include <csl/typeinfo.hpp>
#endif
#if not defined(CSL_TYPEINFO__INCLUDED)
#   error "[csl::ag] csl/ag/formatting/typeinfo.hpp : missing <csl/typeinfo.hpp> (csl::typeinfo). Make it reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#endif

namespace csl::ag::io {

    template <typename T>
    requires requires {
        { csl::typeinfo::type_name<T>::value } -> std::convertible_to<std::string_view>;
    }
    struct type_name<T> : csl::typeinfo::type_name<T>{};
}
