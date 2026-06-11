#include <csl/ag.hpp>

auto main() -> int {

    struct type { const int i = 0; char & c; }; // NOLINT
    char c = 'c';
    auto value = type{ 42, c }; // NOLINT

    static_assert(std::same_as<
        const int,
        csl::ag::element_t<0, std::remove_cvref_t<decltype(value)>>
    >);
    static_assert(std::same_as<
        char&,
        csl::ag::element_t<1, std::remove_cvref_t<decltype(value)>>
    >);
}
