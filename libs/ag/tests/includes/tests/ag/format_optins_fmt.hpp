#pragma once

// Per-type fmt::formatter opt-ins for the shared test types (tests/types.hpp).
// Include after <csl/ag/formatting/backend/fmt.hpp> and <tests/types.hpp>.
// NOTE: test::ag::types::without_formatter is deliberately absent
//       its absence is what guards against a blanket fmt::formatter reappearing.

template <>
struct fmt::formatter<test::ag::types::field_1>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_1>{};
template <>
struct fmt::formatter<test::ag::types::field_2>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_2>{};
template <>
struct fmt::formatter<test::ag::types::field_3_nested>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_3_nested>{};
template <>
struct fmt::formatter<test::ag::types::field_3_nested_tuplelike>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_3_nested_tuplelike>{};
template <>
struct fmt::formatter<test::ag::types::field_4_nested_range>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_4_nested_range>{};
template <>
struct fmt::formatter<test::ag::types::field_everything>
    : csl::ag::formatting::fmt_formatter<test::ag::types::field_everything>{};
