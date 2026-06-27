#include <csl/typeinfo.hpp> // optional: gives csl::ag::io::typenamed clean type names (e.g. "int")
#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT 1
#include <csl/ag.hpp>
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

    // legacy alternative: CSL_AG__ENABLE_IOSTREAM_SUPPORT
    // std::cout << "value: " << format_options << value << '\n';

    // other alternative: CSL_AG__ENABLE_FMT_SUPPORT
    // fmt::println("{:xit}", value)
}
