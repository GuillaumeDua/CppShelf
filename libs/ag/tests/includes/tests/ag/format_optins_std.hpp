#pragma once

// Per-type std::formatter opt-ins for the shared test types (tests/types.hpp).
// Include after <csl/ag/formatting/backend/std_format.hpp> and <tests/types.hpp>.
// NOTE: test::ag::types::without_formatter is deliberately absent.
//       its absence is what guards against a blanket std::formatter reappearing.

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
template <>
struct std::formatter<test::ag::types::field_1>
    : csl::ag::formatting::std_formatter<test::ag::types::field_1>{};
template <>
struct std::formatter<test::ag::types::field_2>
    : csl::ag::formatting::std_formatter<test::ag::types::field_2>{};
template <>
struct std::formatter<test::ag::types::field_3_nested>
    : csl::ag::formatting::std_formatter<test::ag::types::field_3_nested>{};
template <>
struct std::formatter<test::ag::types::field_3_nested_tuplelike>
    : csl::ag::formatting::std_formatter<test::ag::types::field_3_nested_tuplelike>{};
template <>
struct std::formatter<test::ag::types::field_4_nested_range>
    : csl::ag::formatting::std_formatter<test::ag::types::field_4_nested_range>{};
template <>
struct std::formatter<test::ag::types::field_everything>
    : csl::ag::formatting::std_formatter<test::ag::types::field_everything>{};
// NOLINTEND(cert-dcl58-cpp)
