// iword cross-TU/ODR contract: details::mode_index() must return the same xalloc slot in every TU,
// so manipulator state applied in one TU is seen by prints in another.
//
// NOTE(regression): an internal-linkage - static - mode_index gave each TU its own slot
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/ostream.hpp>

#include <sstream>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::ag::cross_tu {
    // defined in ostream_iword_cross_tu_remote.cpp
    auto iword_slot() -> int;
    void set_indented(std::ostream & os);
}

namespace {
    struct A { int i; char c; };
}

TEST_CASE("ostream: iword slot is program-wide", "[ag][formatting][iostream][cross_tu]") {
    CHECK(test::ag::cross_tu::iword_slot() == csl::ag::formatting::details::mode_index());
}

TEST_CASE("ostream: manipulator applied in another TU", "[ag][formatting][iostream][cross_tu]") {
    std::ostringstream os;
    test::ag::cross_tu::set_indented(os);

    using namespace csl::ag::formatting;
    os << A{ .i = 42, .c = 'x' };
    CHECK(os.str() ==
R"({
    42,
    'x'
})");
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
