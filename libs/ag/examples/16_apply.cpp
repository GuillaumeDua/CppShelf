#include <csl/ag.hpp>

auto main() -> int {

    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    auto result = csl::ag::apply([](auto && ... fields){
        return (static_cast<float>(fields) + ...);
    }, value);
    static_assert(std::same_as<float, decltype(result)>);
}
