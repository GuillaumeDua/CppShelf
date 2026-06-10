#include <csl/ag.hpp>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13f };

    auto result = csl::ag::apply([](auto && ... fields){
        return (fields + ...);
    }, value);
    static_assert(std::same_as<float, decltype(result)>);
}
