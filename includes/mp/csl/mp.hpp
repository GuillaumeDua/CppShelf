#pragma once

#include <type_traits>
#include <utility>
#include <tuple>
// todo : remove dependency on std::tuple and std::integer_sequence
//  then add a compile-time option to extend csl::mp to tuple (get, etc.) if required
#include <algorithm>
#include <iterator>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)                     // NOLINT(cppcoreguidelines-macro-usage)
#define static_dependent_error(message) static_assert([](){ return false; }(), message) // NOLINT(cppcoreguidelines-macro-usage)

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
    using reverse_integer_sequence_t = typename reverse_integer_sequence<T, values...>::type;

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
    // pack
    template <typename ... Ts>
    struct pack{};

    // is_pack
    template <typename T>
    struct is_pack : std::false_type{};
    template <typename ... Ts>
    struct is_pack<pack<Ts...>> : std::true_type{};
    template <typename T>
    constexpr bool is_pack_v = is_pack<T>::value;
    template <typename T>
    concept PackType = is_pack_v<T>;

    // is_same_template
    template <typename, typename>
    struct is_same_template : std::false_type{};
    template <template <typename...> typename T, typename ... Ts, typename ... Us>
    struct is_same_template<T<Ts...>, T<Us...>> : std::true_type{};
    template <typename T, typename U>
    constexpr bool is_same_template_v = is_same_template<T, U>::value;
    template <typename T, typename U>
    concept SameTemplateAs = is_same_template<T, U>::value;

    // is_instance_of
    template <template <typename ...> typename pack_type, typename T>
    struct is_instance_of : std::false_type{};
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct is_instance_of<pack_type, pack_type<Ts...>> : std::true_type{};
    template <template <typename ...> typename pack_type, typename T>
    constexpr bool is_instance_of_v = is_instance_of<pack_type, T>::value;
    template <template <typename ...> typename pack_type, typename T>
    concept InstanceOf = is_instance_of_v<pack_type, T>;
}
// todo :
//  check `std::make_index_sequence` performances
//  if not acceptable
//      poc, remove & replace with smthg more efficient
namespace csl::mp::details {

    template <std::size_t I, typename T>
    struct element {
        constexpr static std::size_t index = I;
        using type = T;

        // index-to-type mapping
        constexpr static element<I, T> deduce_type(std::integral_constant<std::size_t, I>);
        // type-to-index mapping
        constexpr static element<I, T> deduce_index(std::type_identity<T>);
    };

    template <typename ... Ts>
    struct elements_pack : Ts... {
        using Ts::deduce_type...;
        using Ts::deduce_index...;

        template <std::size_t I>
        using nth_ = decltype(deduce_type(std::integral_constant<std::size_t, I>{}));
        template <typename T>
        using index_of_ = decltype(deduce_index(std::type_identity<T>{}));
    };

    template <typename ... Ts>
    struct make_elements_pack {
        using type = decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return elements_pack<element<indexes, Ts>...>{};
        }(std::make_index_sequence<sizeof...(Ts)>{}));
    };
    template <typename ... Ts>
    using make_elements_pack_t = typename make_elements_pack<Ts...>::type;

    #if false
    // required for previous `nth_t` implementation
    template <std::size_t I>
    struct is_element_match {
        template <typename>
        struct type : std::false_type{};
        template <typename T>
        struct type<element<I, T>> : std::true_type{};
    };
    #endif
}
// abstraction on (ttps...|pack<ttps...>)
namespace csl::mp {

    // front / head / first_of
    template <typename>
    struct front;
    template <template <typename ...> typename pack, typename T, typename ... Ts>
    struct front<pack<T, Ts...>> : std::type_identity<T>{};
    template <typename T>
    using front_t = typename front<T>::type;

    // filters
    // todo : remove dependency to std::tuple
    //  using csl::mp::push_back instead
    //  or Ts... -> pack<pack<T0>, pack<>, pack<T2> ...> => flatten
    template <template <typename> typename, typename>
    struct filters;
    template <template <typename> typename filter_type, template <typename...> typename pack, typename ... Ts>
    requires requires { ((filter_type<Ts>::value) and ...); }
    struct filters<filter_type, pack<Ts...>>
        : std::type_identity<decltype(std::tuple_cat(std::conditional_t< // todo : cat instead
            filter_type<Ts>::value,
            std::tuple<Ts>,
            std::tuple<>
        >{}...))>{}; // todo : repack tuple into pack

    template <template <typename> typename filter_type, typename pack>
    using filters_t = typename filters<filter_type, pack>::type;

    // nth element
    //  wip benchmarks : https://www.build-bench.com/b/gysn2DDajc38cdeUbNHnjmtKKvA
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
    // Indexed pack
    template <typename>
    struct pack_element;
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct pack_element<pack_type<Ts...>> : details::make_elements_pack_t<Ts...>{};

    template <std::size_t I, typename T>
    using pack_element_t = typename  pack_element<T>::template nth_<I>::type;
    template <std::size_t I, typename T>
    constexpr std::size_t pack_element_v = I /*Note : equivalent to pack_element<I, T>::value;*/;

    template <std::size_t I, typename T>
    using nth_t = pack_element_t<I, T>;
    template <std::size_t I, typename T>
    constexpr std::size_t nth_v = pack_element_v<I, T>;

    template <typename T, typename pack_type>
    constexpr std::size_t index_of_v = pack_element<pack_type>::template index_of_<T>::index;
    // rindex_of -> index_of_v<T, reverse<pack_type>>
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

    // cat / append
    template <typename, typename...>
    struct cat;
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, Us...> : std::type_identity<pack<Ts..., Us...>>{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, pack<Us...>> : std::type_identity<pack<Ts..., Us...>>{};
    template <typename pack, typename... ttps>
    using cat_t = cat<pack, ttps...>;

    // unfold_to
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_to : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_to<destination, from<Ts...>> : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_to_t = typename unfold_to<destination, from...>::type;

    // flat_cat
    template <typename, typename ...>
    struct flat_cat;
    template <template <typename...> typename pack_type, typename ... Ts, typename ... rest>
    struct flat_cat<pack_type<Ts...>, rest...>
    : std::type_identity<
        unfold_to_t<pack_type, decltype(std::tuple_cat(
            std::tuple<Ts...>{},
            std::conditional_t<
                is_instance_of_v<pack_type, rest>,
                unfold_to_t<std::tuple, rest>,
                std::tuple<rest>
            >{}...
        ))>
    >{};
    template <typename pack_type, typename ... Ts>
    using flat_cat_t = typename flat_cat<pack_type, Ts...>::type;

    // flat_merge : flat_cat + deduplication
    // todo

    // flatten_once
    template <typename T>
    struct flatten_once
    : std::type_identity<T>{};
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct flatten_once<pack_type<Ts...>>
    : std::type_identity<
        unfold_to_t<pack_type, decltype(std::tuple_cat(
            std::conditional_t<
                is_instance_of_v<pack_type, Ts>,
                unfold_to_t<std::tuple, Ts>,
                std::tuple<Ts>
            >{}...
        ))>
    >{};
    template <typename T>
    using flatten_once_t = typename flatten_once<T>::type;

    // flatten
    template <typename T>
    struct flatten;
    template <typename T>
    requires (std::same_as<T, flatten_once_t<T>>)
    struct flatten<T>
    : std::type_identity<T>{};
    template <typename T>
    struct flatten
    : std::type_identity<typename flatten<flatten_once_t<T>>::type>{};
    template <typename T>
    using flatten_t = typename flatten<T>::type;

    // reverse
    template <typename>
    class reverse;
    template <template <typename ...> typename pack_type, typename ... Ts>
    class reverse<pack_type<Ts...>> {
        constexpr static auto impl() {
            return [](){
                // todo : reverse_sequence, type_at
            }();
        }
    };
    template <typename T>
    using reverse_t = typename reverse<T>::type;

    #ifdef false
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
    #endif

    // at
    // todo

    // transform
    template <typename T, template <typename> typename trait>
    struct transform;
    template <typename ... Ts, template <typename> typename trait>
    struct transform<pack<Ts...>, trait> : std::type_identity<pack<trait<Ts>...>>
    {};
    template <typename T, template <typename> typename trait>
    using transform_t = typename transform<T, trait>::type;

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
    class deduplicate;
    template <template <typename...> typename type_pack, typename ... Ts>
    class deduplicate<type_pack<Ts...>> {

        using tuple_type = typename deduplicate_impl<Ts...>::type;
    public:
        using type = unfold_to_t<type_pack, tuple_type>;
    };
    template <typename T>
    using deduplicate_t = typename deduplicate<T>::type;
}

#undef fwd
#undef static_dependent_error

// wip : https://godbolt.org/z/TfMqM5TaG
// wip, benchemarked : https://godbolt.org/z/fj4z8sjzh
// wip bench : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
