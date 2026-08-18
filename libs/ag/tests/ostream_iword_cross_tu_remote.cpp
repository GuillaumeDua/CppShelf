// iword cross-TU contract - remote TU: see ostream_iword_cross_tu.cpp
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/ostream.hpp>

#include <ostream>

namespace test::ag::cross_tu {
    auto iword_slot() -> int { return csl::ag::formatting::details::mode_index(); }
    void set_indented(std::ostream & os) { os << csl::ag::formatting::indented; }
}
