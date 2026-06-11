#include <csl/ag.hpp>
#include <utility>

auto main() -> int {

    struct type {
        int value; int & lvalue; const int & const_lvalue; int && rvalue; // NOLINT(*-avoid-const-or-ref-data-members)
        [[nodiscard]] static auto make(int & i){ return type{ .value=i, .lvalue=i, .const_lvalue=i, .rvalue=std::move(i) }; }
    };
    int i = 42; // NOLINT(*-avoid-magic-numbers)

    { // rvalue source: non-ref fields acquire rvalue-ref qualification
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(type::make(i));

        static_assert(std::same_as<
            decltype(view),
            csl::ag::view_t<type&&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<int&&, int&, const int&, int&&>
        >);
    }

    { // const-lvalue source: non-ref fields acquire const-lvalue-ref qualification
        const auto & value = type::make(i); // lifetime extension
        [[maybe_unused]] auto view = csl::ag::to_tuple_view(value);

        static_assert(std::same_as<
            decltype(view),
            csl::ag::view_t<const type&>
        >);
        static_assert(std::same_as<
            decltype(view),
            std::tuple<const int&, int&, const int&, int&&>
        >);
    }
}
