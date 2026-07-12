#include <csl/ag.hpp>
#include <csl/ag/formatting/format.hpp>   // opt-in: std::formatter support
#include <csl/typeinfo.hpp>               // bridge prerequisite (explicit, for godbolt raw-URL include order)
#include <csl/ag/formatting/typeinfo.hpp> // opt-in: gives csl::ag::io::typenamed clean type names (e.g. "int")
#include <iostream> // std::print might not be available yet: use `std::cout << std::format(...)`

struct S { char c; int i; };

static_assert(
    csl::ag::concepts::aggregate<S> and
    csl::ag::size_v<S> == 2
);
static_assert(std::same_as<char, csl::ag::element_t<0, S>>);
static_assert(std::same_as<int,  csl::ag::element_t<1, S>>);

auto main() -> int {
    auto value = S{ .c='A', .i=41 }; // NOLINT
    ++csl::ag::get<1>(value);

    using namespace csl::ag::io;
    constexpr auto format_options = indexed | typenamed | indented;
    std::cout << std::format("{}", value | format_options); // equivalent to std::println("{:xit}", value)

    // alternative: #include <csl/ag/formatting/ostream.hpp>
    // std::cout << "value: " << format_options << value << '\n';

    // alternative: #include <csl/ag/formatting/fmt.hpp>
    // fmt::println("{:xit}", value)
}
