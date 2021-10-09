#pragma once

#include <type_traits>
#include <utility>
#include <tuple>
// todo : remove dependency on std::tuple and std::integer_sequence
//  then add a compile-time option to extend csl::mp to tuple (get, etc.) if required
#include <algorithm>
#include <iterator>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)
#define static_dependent_error(message) static_assert([](){ return false; }(), message)

// cpp shelf library : metaprogramming
// sequences
namespace csl::mp::seq {
    // reverse_integer_sequence
    template <typename T, T ... values>
    class reverse_integer_sequence {
        // todo : remove if compilers support lambdas in non-evaluated context
        constexpr static decltype(auto) impl() {
            return []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                constexpr auto values_as_array = std::array<T, sizeof...(values)>{ values... };
                return std::integer_sequence<
                    T,
                    std::get<(sizeof...(values) - 1 - indexes)>(values_as_array)...
                >{};
            }(std::make_index_sequence<sizeof...(values)>());
        }
    public:
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

namespace csl::mp {
    template <typename ... Ts>
    struct pack{};
}
namespace csl::mp::details {

    #pragma region pack element
    template <std::size_t I, typename T>
    struct element {
        constexpr static std::size_t index = I;
        using type = T;
    };

    // todo : remove std::make_index_sequence, replace with smthg more efficient
    template <typename ... Ts>
    struct make_element_pack {
        using type = decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return pack<element<indexes, Ts>...>{};
        }(std::make_index_sequence<sizeof...(Ts)>{}));
    };
    template <typename ... Ts>
    using make_element_pack_t = make_element_pack<Ts...>::type;

    template <std::size_t I>
    struct is_element_match {
        template <typename>
        struct type : std::false_type{};
        template <typename T>
        struct type<element<I, T>> : std::true_type{};
    };
    #pragma endregion

    #pragma region pack indexation details
    // todo : merge with `element`, hide `indice_` + friend
    template <std::size_t I, typename T>
    struct indexed_element {
        constexpr static csl::mp::details::element<I, T> indice_(std::integral_constant<std::size_t, I>);
    };

    template <typename ... Ts>
    struct indexed_elements : Ts... {
        using Ts::indice_...;

        template <std::size_t I>
        using nth_ = decltype(indice_(std::integral_constant<std::size_t, I>{}));
    };

    template <typename ... Ts>
    struct make_indexed_elements {
        using type = decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return indexed_elements<indexed_element<indexes, Ts>...>{};
        }(std::make_index_sequence<sizeof...(Ts)>{}));
    };
    #pragma endregion
}
// abstraction on (ttps...|pack<ttps...>)
namespace csl::mp {

    // front / head / first_of
    template <typename>
    struct front;
    template <template <typename ...> typename pack, typename T, typename ... Ts>
    struct front<pack<T, Ts...>> : std::type_identity<T>{};
    template <typename T>
    using front_t = front<T>::type;

    // filters
    // todo : remove dependency to std::tuple
    //  using csl::mp::push_back instead
    //  or Ts... -> pack<pack<T0>, pack<>, pack<T2> ...> => flatten
    template <template <typename> typename, typename>
    struct filters;
    template <template <typename> typename filter_type, template <typename...> typename pack, typename ... Ts>
    requires requires { ((filter_type<Ts>::value) and ...); }
    struct filters<filter_type, pack<Ts...>>
        : std::type_identity<decltype(std::tuple_cat(std::conditional_t< // todo : append instead
            filter_type<Ts>::value,
            std::tuple<Ts>,
            std::tuple<>
        >{}...))>{}; // todo : repack tuple into pack

    template <template <typename> typename filter_type, typename pack>
    using filters_t = filters<filter_type, pack>::type;

    // nth element
    //  wip benchmarks : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
    #if false
    // disabled, as csl::mp::filters_t relies on tuple for now
    template <std::size_t index, typename>
    struct nth_element;
    template <std::size_t index, template <typename ...> typename pack, typename ... Ts>
    requires (index < sizeof...(Ts))
    struct nth_element<index, pack<Ts...>> : front<
        filters_t<
            details::is_element_match<index>::template type,
            details::make_element_pack_t<Ts...>
        >
    >{};
    template <std::size_t index, typename pack>
    using nth_element_t = nth_element<index, pack>::type;
    template <std::size_t index, typename pack>
    using nth_t = nth_element_t<index, pack>::type;
    template <std::size_t index, typename pack>
    constexpr std::size_t nth_v = nth_element_t<index, pack>::value;
    #else
    // overload index table
    template <std::size_t, typename>
    struct pack_element;
    template <std::size_t I, template <typename ...> typename pack_type, typename ... Ts>
    struct pack_element<I, pack_type<Ts...>> : details::make_indexed_elements<Ts...>::type::nth_<I>{};

    template <std::size_t I, typename T>
    using pack_element_t = pack_element<I, T>::type;
    template <std::size_t I, typename T>
    constexpr std::size_t pack_element_v = I /*pack_element<I, T>::value;*/;

    template <std::size_t I, typename T>
    using nth_t = pack_element_t<I, T>;
    template <std::size_t I, typename T>
    constexpr std::size_t nth_v = pack_element_v<I, T>;
    #endif

    // todo : index_of<T, pack_t>

    // todo : no, use csl::mp instead
    //  need to define csl::Pack concept
    template <typename T>
    concept TupleType = requires { std::tuple_size_v<T>; };

    // get<index>
    template <std::size_t index, typename ... Ts>
    constexpr decltype(auto) get(Ts && ... values) noexcept {
        return std::get<index>(std::tuple<decltype(values)...>{ fwd(values)...});
    }
    template <std::size_t index, TupleType T>
    constexpr decltype(auto) get(T && value) noexcept {
        return std::get<index>(value);
    }
    template <std::size_t index, typename T, auto ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...> && value) noexcept {
        return mp::seq::get<index>(fwd(value));
    }

    // todo : remove
    // deprecated, use `nth_t` instead
    // template <std::size_t index, typename... Ts>
    // struct type_at : std::tuple_element<index, std::tuple<Ts...>>{};
    // template <std::size_t index, template <typename...> typename pack, typename ...Ts>
    // struct type_at<index, pack<Ts...>> : std::tuple_element<index, std::tuple<Ts...>>{};
    // template <std::size_t index, typename... Ts>
    // using type_at_t = type_at<index, Ts...>::type;

    #pragma region is_template
    // is_template : ttps
    template <typename ...>
    struct is_template_with_ttps : std::false_type{};
    template <template <typename...> typename pack_type, typename ... Ts>
    struct is_template_with_ttps<pack_type<Ts...>> : std::true_type{};
    template <typename ... Ts>
    constexpr bool is_template_with_ttps_v = is_template_with_ttps<Ts...>::value;

    template <typename T>
    concept TemplateType_ttps = is_template_with_ttps_v<T>;

    // is_template : nttps
    template <typename ...>
    struct is_template_with_nttps : std::false_type{};
    template <template <auto...> typename pack_type, auto ... values>
    struct is_template_with_nttps<pack_type<values...>> : std::true_type{};
    template <typename ... Ts>
    constexpr bool is_template_with_nttps_v = is_template_with_nttps<Ts...>::value;

    template <typename T>
    concept TemplateType_nttps = is_template_with_nttps_v<T>;

    // is_template
    template <typename ... Ts>
    struct is_template {
        constexpr static bool value = is_template_with_ttps_v<Ts...> or is_template_with_nttps_v<Ts...>;
    };
    template <typename ... Ts>
    constexpr bool is_template_v = is_template<Ts...>::value;

    template <typename T>
    concept TemplateType = is_template_v<T>;

    #pragma endregion

    // size
    template <typename ... ttps>
    struct size : std::integral_constant<std::size_t, sizeof...(ttps)>{};
    template <template <typename...> typename pack, typename ... ttps>
    struct size<pack<ttps...>> : size<ttps...>{};
    template <typename ... ttps>
    constexpr std::size_t size_v = size<ttps...>::value;

    // contains
    template <typename T, typename ... ttps>
    struct contains : std::bool_constant<(std::is_same_v<T, ttps> or ...)>{};
    template <typename T, template <typename...> typename pack, typename ... ttps>
    struct contains<T, pack<ttps...>> : contains<T, ttps...>{};
    template <typename T, typename ... ttps>
    constexpr bool contains_v = contains<T, ttps...>::value;

    // count - occurences of T in ttps...
    template <typename T, typename ... ttps>
    struct count : std::integral_constant<std::size_t, (std::size_t{std::is_same_v<T, ttps>} + ...)>{};
    template <typename T, template <typename...> typename pack, typename ... ttps>
    struct count<T, pack<ttps...>> : count<T, ttps...>{};
    template <typename T, typename ... ttps>
    constexpr std::size_t count_v = count<T, ttps...>::value;

    // append
    template <typename, typename...>
    struct append;
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct append<pack<Ts...>, Us...> : std::type_identity<pack<Ts..., Us...>>{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct append<pack<Ts...>, pack<Us...>> : std::type_identity<pack<Ts..., Us...>>{};
    template <typename pack, typename... ttps>
    using append_t = append<pack, ttps...>;

    // merge
    //  todo : add if not duplicates ?

    // repack_into
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_to : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_to<destination, from<Ts...>> : unfold_to<destination, Ts...>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_to_t = unfold_to<destination, from...>::type;

    // reverse
    template <typename>
    struct reverse;
    template <template <typename ...> typename pack_type, typename ... Ts>
    class reverse<pack_type<Ts...>> {
        constexpr static auto impl() {
            return [](){
                // todo : reverse_sequence, type_at
            }();
        }
    };
    template <typename T>
    using reverse_t = reverse<T>::type;

    // rindex_of
    template <typename T, typename ... Ts>
    struct rindex_of {
        constexpr static std::size_t value = rindex_of<T, unfold_to_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename...> typename pack, typename ... Ts>
    struct rindex_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            ((indexes, pos = (std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "rindex_of : not found");
    };
    template <typename T, typename ... Ts>
    constexpr std::size_t rindex_of_v = rindex_of<T, Ts...>::value;

    // index_of
    template <typename T, typename ... Ts>
    struct index_of {
        constexpr static std::size_t value = index_of<T, unfold_to_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename ...> typename pack, typename ... Ts>
    struct index_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            ((indexes, pos = (pos == std::string::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "index_of : not found");
    };
    template <typename T, typename ... Ts>
    constexpr std::size_t index_of_v = index_of<T, Ts...>::value;

    template <typename T, typename ... Ts>
    using first_index_of = index_of<T, Ts...>;
    template <typename T, typename ... Ts>
    constexpr std::size_t  first_index_of_v = first_index_of<T, Ts...>::value;

    template <typename T, typename ... Ts>
    using last_index_of = std::integral_constant<
        std::size_t,
        size_v<Ts...> //- 1 - rindex_of_v<T, Ts...>
    >;
    template <typename T, typename ... Ts>
    constexpr std::size_t last_index_of_v = last_index_of<T, Ts...>::value;

    // at

    // deduplicate
    //  todo : refactor :
    //      contains => index_of not_eq rindex_of
    template <typename ...>
    struct deduplicate_impl;
    template <typename T>
    struct deduplicate_impl<T> {
        using type = std::tuple<T>;
    };
    template <typename first, typename ... rest>
    struct deduplicate_impl<first, rest...> {
        using type = decltype(std::tuple_cat(
            std::conditional_t<
                contains_v<first, std::tuple<rest...>>,
                std::tuple<>,
                std::tuple<first>
            >{},
            typename deduplicate_impl<rest...>::type{}
        ));
    };

    template <typename T>
    struct deduplicate;
    template <template <typename...> typename type_pack, typename ... Ts>
    class deduplicate<type_pack<Ts...>> {

        using tuple_type = typename deduplicate_impl<Ts...>::type;
    public:
        using type = unfold_to_t<type_pack, tuple_type>;
    };
    template <typename T>
    using deduplicate_t = deduplicate<T>::type;
}

#undef fwd
#undef static_dependent_error

// wip : https://godbolt.org/z/TfMqM5TaG
// wip, benchemarked : https://godbolt.org/z/fj4z8sjzh
// wip bench : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
