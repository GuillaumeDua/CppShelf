#include <csl/ag.hpp>
#include <tuple>

void do_stuff(csl::ag::concepts::structured_bindable auto && value) {
    using value_type = std::remove_cvref_t<decltype(value)>;
    using namespace csl::ag::tuplelike;

    constexpr auto sz = size_v<value_type>;

    const auto process = [&]<std::size_t index>(){
        [[maybe_unused]] auto && field = get<index>(std::forward<decltype(value)>(value));
        using field_type = element_t<index, value_type>;
        (void)static_cast<field_type *>(nullptr);
    };

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((process.template operator()<indexes>()), ...);
    }(std::make_index_sequence<sz>{});
}

auto main() -> int {
    struct A { int i; float f; };
    do_stuff(A{ .i = 1, .f = 2.f });
    do_stuff(std::tuple<int, float>{ 1, 2.f });
}
