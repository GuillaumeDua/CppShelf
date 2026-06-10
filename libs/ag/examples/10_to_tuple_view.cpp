#include <csl/ag.hpp>
#include <utility>

auto main() -> int {
    struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };
    int i = 42;

    { // rvalue source: non-ref fields acquire rvalue-ref qualification
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(type{ i, i, i, std::move(i) });

        static_assert(std::same_as<
            decltype(view),
            csl::ag::tuple_view_t<type&&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<int&&, int&, const int&, int&&>
        >);
    }

    { // const-lvalue source: non-ref fields acquire const-lvalue-ref qualification
        const auto & value = type{ i, i, i, std::move(i) };
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(value);

        static_assert(std::same_as<
            decltype(view),
            csl::ag::tuple_view_t<const type&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<const int&, int&, const int&, int&&>
        >);
    }
}
