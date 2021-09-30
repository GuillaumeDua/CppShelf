#pragma once

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)
#define static_dependent_error(message) static_assert([](){ return false; }(), message)

// cpp shelf library : metaprogramming
namespace csl::mp::seq {
    // reverse_integer_sequence
    template <typename T, T ... values>
    class reverse_integer_sequence {
        constexpr static decltype(auto) impl() {
            return []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                constexpr auto values_as_array = std::array<T, sizeof...(values)>{ values... };
                return std::integer_sequence<
                    T,
                    std::get<(sizeof...(values) - 1 - indexes)>(values_as_array)...
                >{};
            }(std::make_index_sequence<sizeof...(values)>());
        }
        using type = decltype(impl());
    };
    template <typename T, T ... values>
    struct reverse_integer_sequence<std::integer_sequence<T, values...>> : reverse_integer_sequence<T, values...>{};
    template <typename T, std::size_t ... values>
    using reverse_integer_sequence_t = reverse_integer_sequence<T, values...>::type;

    template <std::size_t size>
    using make_reversed_index_sequence = reverse_integer_sequence_t<std::make_index_sequence<size>>;

    // type_of<values...>
    template <auto value, auto ... values>
    requires (std::is_same_v<decltype(value), decltype(values)> and ...)
    struct type_of : std::type_identity<decltype(value)>{};
    template <auto ... values>
    using type_of_t = typename type_of<values...>::value;

    // type_at<index, (integer_seq|values...)>
    //  todo : universal template parameters : (values...|std::integer_seq)
    template <std::size_t index, auto ... values>
    struct value_at : std::integral_constant<
        decltype(std::get<index>(std::declval<std::tuple<decltype(values)...>>(values...))),
        std::get<index>(std::tuple<decltype(values)...>{values...})
    >{};

    // get<index, values...>()
    template <std::size_t index, auto ... values>
    requires requires { type_of_t<values...>{}; }
    constexpr decltype(auto) get() noexcept {
        constexpr auto storage = std::array<type_of_t<values...>, sizeof...(values)>{values...};
        return std::get<index>(storage);
    }
    // get<index>(seq)
    template <std::size_t index, typename T, T ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...>) noexcept {
        return get<index, values...>;
    }
}

#undef fwd
#undef static_dependent_error