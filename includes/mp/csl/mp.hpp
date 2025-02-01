#pragma once
// cpp shelf library : metaprogramming utility
// under MIT License - Copyright (c) 2021 Guillaume Dua "Guss"
// https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

// About [tuples]:
//  tuples are considered as valid if csl::mp::is_tuple is true_type, and contains no duplicates
//  if a given tuple instanciation is not valid, less performant algorithms may be selected

#include <bits/utility.h>
#include <compare>
#include <type_traits>
#include <concepts>
// todo : remove dependency on std::tuple and std::integer_sequence
//  then add a compile-time option to extend csl::mp to tuple (get, etc.) if required
#include <algorithm>
#include <iterator>
#include <compare>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)                     // NOLINT(cppcoreguidelines-macro-usage)
#define static_dependent_error(message) static_assert([](){ return false; }(), message) // NOLINT(cppcoreguidelines-macro-usage)

// [WIP] refactoring => reorder/sort
//  sequences::* : op on sequences
//  seq types    : sequences definitions
//  tuple
//  mp::pack::* : ttps...| tttp<ttps...> -> only mp::*<tttp<ttps...>>
//
// [WIP] range-like API for sequence, tuple
//  std::make_index_sequence<4> | reverse | drop(1) | take(2) => std::integer_sequence<std::size_t, 3, 2>

// sequences
#include <array>
namespace csl::mp::seq::type_traits {
    // is_sequence
    template <typename T>
    struct is_sequence : std::false_type{};
    template <typename T, T ... values>
    struct is_sequence<std::integer_sequence<T, values...>> : std::true_type{};
    template <typename T>
    constexpr static inline auto is_sequence_v = is_sequence<T>::value;
}
namespace csl::mp::seq::concepts {
    template <typename T>
    concept sequence = type_traits::is_sequence_v<T>;
}
namespace csl::mp::seq {

    // to_tuplelike
    template <typename T>
    struct to_tuplelike;
    template <typename T, T ... values>
    struct to_tuplelike<std::integer_sequence<T, values...>>{
        using type = std::array<T, sizeof...(values)>;
        constexpr static inline auto value = type{ values... };
    };
    template <typename T>
    constexpr static inline auto to_tuplelike_t = to_tuplelike<T>::type;
    template <typename T>
    constexpr static inline auto to_tuplelike_v = to_tuplelike<T>::value;

    // size | pref. seq::size()
    template <typename T>
    struct size;
    template <typename T, T ... values>
    struct size<std::integer_sequence<T, values...>>
    : std::integral_constant<std::size_t, sizeof...(values)>
    {};

    // at<index>
    template <std::size_t, typename>
    struct at;
    template <std::size_t index, typename T, T ... values>
    struct at<index, std::integer_sequence<T, values...>> : std::integral_constant<
        T,
        std::get<index>(
            to_tuplelike_v<std::integer_sequence<T, values...>>
        )
    >{};
    template <std::size_t index, typename T>
    constexpr static inline auto at_v = at<index, T>::value;

    // reverse
    template <typename T>
    struct reverse;
    template <concepts::sequence T>
    struct reverse<T> : std::type_identity<
        decltype(
            []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return std::integer_sequence<
                    typename T::value_type,
                    at_v<T::size() - 1 - indexes, T>...
                >{};
            }(std::make_index_sequence<T::size()>{})
        )
        // if 0..N: std::integer_sequence<T, (sizeof...(values) - 1 - values)...>
    >{};
    // template <typename T, T ... values>
    // struct reverse<std::integer_sequence<T, values...>> : std::type_identity<
    //     decltype(
    //         []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //             constexpr auto storage = std::array{ values... };
    //             return std::integer_sequence<
    //                 T,
    //                 std::get<(sizeof...(values) - 1 - indexes)>(storage)...
    //             >{};
    //         }(std::make_index_sequence<sizeof...(values)>{})
    //     )
    //     // if 0..N: std::integer_sequence<T, (sizeof...(values) - 1 - values)...>
    // >{};
    template <typename T>
    using reverse_t = reverse<T>::type;
    template <std::size_t I>
    using make_reverse_index_sequence = reverse_t<std::make_index_sequence<I>>;

    // type_of<values...>
    template <auto value, auto ... values>
    requires (std::is_same_v<decltype(value), decltype(values)> and ...)
    struct type_of : std::type_identity<decltype(value)>{};
    template <auto ... values>
    using type_of_t = typename type_of<values...>::value;
    
    // get<index>(seq)
    template <std::size_t index, typename T, T ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...>) noexcept {
        return at_v<index, std::integer_sequence<T, values...>>;
    }
}

// ---

// csl::mp::tuple
//  TODO(@Guss): structured-binding (std::tuple_size, std::tuple_element)
//  TODO(@Guss): concepts: tuple_interface, empty_tuple, etc.
//  TODO(@Guss): apply
//  WIP:  ==, not_eq
//  WIP: user-defined deduction guide
//  WIP: fix build CI

namespace csl::mp {
    template <std::size_t I>
    using index = std::integral_constant<std::size_t, I>;

#if defined(__cpp_lib_type_identity)
    template <typename T>
    using type_identity = typename std::type_identity<T>;
#else
    template <typename T>
    struct type_identity{ using type = T; };
#endif

    template <typename T>
    struct is_int : std::is_same<T, int>{};

    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_front {
        template <typename ... Us>
        using type = trait<Ts..., Us...>;
    };
    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_back {
        template <typename ... Us>
        using type = trait<Us..., Ts...>;
    };
}
namespace csl::mp::details::compare {

    // emulate __detail::__synth3way_t
    constexpr auto synth_three_way = []<class T, class U>(const T& t, const U& u)
    requires requires
        {
            { t < u } -> std::convertible_to<bool>;
            { u < t } -> std::convertible_to<bool>;
        }
    {
        if constexpr (std::three_way_comparable_with<T, U>)
            return t <=> u;
        else
        {
            if (t < u)
                return std::weak_ordering::less;
            if (u < t)
                return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }
    };
    template <class T, class U = T>
    using synth_three_way_result = decltype(synth_three_way(std::declval<T&>(), std::declval<U&>()));
}
namespace csl::mp::details {

    template <std::size_t I, typename T>
    struct tuple_element {
        constexpr static std::size_t index = I;
        using type = T;

        // index-to-type mapping
        constexpr static tuple_element<I, T> deduce_type(mp::index<I>);
        // type-to-index mapping
        constexpr static tuple_element<I, T> deduce_index(mp::type_identity<T>);
    };


    // QUESTION: or storage<tuple_element<index, T>>
    template <std::size_t index, typename T>
    struct tuple_element_storage {

        T value;

        // template <std::size_t I, std::equality_comparable_with<T> U>
        // constexpr auto operator==(const tuple_element_storage<I, U> & other) const
        // noexcept(noexcept(value == other.value))
        // {
        //     return value == other.value;
        // }
        // template <std::size_t I, std::three_way_comparable_with<T> U>
        // friend constexpr auto operator<=>(
        //     const tuple_element_storage & lhs,
        //     const tuple_element_storage<I, U> & rhs)
        // noexcept(noexcept(lhs.value <=> rhs.value))
        // {
        //     return lhs.value <=> rhs.value;
        // }

        // template <std::size_t I, typename U>
        // constexpr auto operator<=>(const tuple_element_storage<I, U> & other) const
        // // noexcept(noexcept(value <=> other.value))
        // -> std::common_comparison_category_t<compare::synth_three_way_result<T, U>>
        // {
        //     return compare::synth_three_way(value, other.value);
        // }
    };

    template <typename indexes, typename ... Ts>
    struct tuple_storage{
        static_assert([](){ return false; }(), "csl::mp::details::tuple_storage: ill-formed");
    };
    // WIP: explicit, noexcept, requires
    template <std::size_t ... indexes, typename ... Ts>
    struct tuple_storage<std::index_sequence<indexes...>, Ts...>
    : tuple_element_storage<indexes, Ts>...
    {
        template <typename index_seq, typename ... Us>
        friend struct tuple_storage;

        template <typename ... Us>
        constexpr
        explicit(not (true and ... and std::convertible_to<Us&&, Ts>))
        tuple_storage(Us && ... args)
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, Us>))
        requires (true and ... and std::constructible_from<Ts, decltype(fwd(args))>)
        : tuple_element_storage<indexes, Ts>{ fwd(args) }...
        {}

        template <typename seq, typename ... Us>
        explicit constexpr tuple_storage(tuple_storage<seq, Us...> && other)
        requires (true and ... and std::constructible_from<Ts, Us>)
        : tuple_element_storage<indexes, Ts>{
            std::forward<tuple_element_storage<indexes, Us>>(fwd(other)).value
        }...
        {}
        template <typename seq, typename ... Us>
        explicit constexpr tuple_storage(const tuple_storage<seq, Us...> & other)
        requires (true and ... and std::constructible_from<Ts, Us>)
        : tuple_element_storage<indexes, Ts>{
            std::forward<tuple_element_storage<indexes, Us>>(fwd(other)).value
        }...
        {}

        constexpr tuple_storage() = default;
        constexpr ~tuple_storage() = default;
        constexpr tuple_storage(tuple_storage&&) noexcept = default;
        constexpr tuple_storage(const tuple_storage &) = default;
        constexpr tuple_storage & operator=(tuple_storage &&) noexcept = default;
        constexpr tuple_storage & operator=(const tuple_storage &) = default;

        // WIP: not default, but every instances. + friends
        // constexpr bool operator==(const tuple_storage &) const = default;
        // constexpr auto operator<=>(const tuple_storage &) const = default;

        // template <std::equality_comparable_with<Ts> ... Us>
        // constexpr bool operator==(const tuple_storage<std::index_sequence<indexes...>, Us...> &) const {
        //     return true;
        // }
        // template <std::equality_comparable_with<Ts> ... Us>
        // constexpr auto operator<=>(const tuple_storage<std::index_sequence<indexes...>, Us...> &) const {
        //     return std::strong_ordering::equal;
        // }
    };

    template <typename>
    struct tuple_storage_accessor;
    template <std::size_t index, typename T>
    struct tuple_storage_accessor<tuple_element<index, T>> : std::type_identity<tuple_element_storage<index, T>>{};
    template <typename T>
    using tuple_storage_accessor_t = tuple_storage_accessor<T>::type;

    template <typename ... Ts>
    struct tuple_impl : Ts... {
    // tuple-element indexing
        using Ts::deduce_type...;
        using Ts::deduce_index...;

        template <std::size_t I>
        using nth_ = decltype(deduce_type(index<I>{}));
        template <typename T>
        using by_type_ = decltype(deduce_index(type_identity<T>{}));
    };
    template <>
    struct tuple_impl<>{};

    template <typename sequence_type, typename ... Ts>
    struct make_tuple;
    template <std::size_t ... indexes, typename ... Ts>
    struct make_tuple<std::index_sequence<indexes...>, Ts...> : type_identity<
        typename mp::details::tuple_impl<mp::details::tuple_element<indexes, Ts>...>
    >{};
    template <typename sequence_type, typename ... Ts>
    using make_tuple_t = typename make_tuple<sequence_type, Ts...>::type;
}
namespace csl::mp {

    template <typename ... Ts>
    struct tuple : mp::details::make_tuple_t<
        std::make_index_sequence<sizeof...(Ts)>,
        Ts...
    >{
        template <typename ... Us> friend struct tuple;

        using type = tuple<Ts...>;
        constexpr static auto size = sizeof...(Ts);
        using index_sequence_type = std::make_index_sequence<size>;
        using storage_type = details::tuple_storage<index_sequence_type, Ts...>;

    // storage
        constexpr tuple() = default;
        constexpr tuple(const tuple &) = default;
        constexpr tuple(tuple &&) noexcept((true and ... and std::is_nothrow_move_constructible_v<Ts>)) = default;
        constexpr ~tuple() = default;

        template <std::convertible_to<Ts> ... Us>
        constexpr auto & operator=(tuple<Us...> && other)
        noexcept((true and ... and std::is_nothrow_convertible_v<Us&&, Ts&>))
        {
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return ((get<indexes>() = fwd(other).template get<indexes>()), ...);
            }(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }
        template <std::convertible_to<Ts> ... Us>
        constexpr auto & operator=(tuple<Us...> & other)
        noexcept((true and ... and std::is_nothrow_convertible_v<Us&, Ts&>))
        {
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return ((get<indexes>() = fwd(other).template get<indexes>()), ...);
            }(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }

        explicit constexpr tuple(auto && ... args)
        requires (std::constructible_from<Ts, decltype(fwd(args))> and ...)
        : storage{ fwd(args)... }
        {}
        template <typename ... Us>
        constexpr tuple(tuple<Us...> && other)
        requires (true and ... and std::constructible_from<Ts, Us&&>)
        : storage{ fwd(other).storage }
        {}
        template <typename ... Us>
        constexpr tuple(const tuple<Us...> & other)
        requires (true and ... and std::constructible_from<Ts, const Us&>)
        : storage{ fwd(other).storage }
        {}

        // compare
        template <typename ... Us>
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::equality_comparable_with<Ts, Us>)
        constexpr auto operator==(const tuple<Us...> & other) const {
            return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return (true and ... and (get<indexes>() == other.template get<indexes>()));
            }(std::make_index_sequence<sizeof...(Ts)>{});
        }
        template <typename ... Us>
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::three_way_comparable_with<Ts, Us>)
        constexpr auto operator<=>(const tuple<Us...> & other) const
        -> std::common_comparison_category_t<std::__detail::__synth3way_t<Ts, Us>...>
        {
            using _Cat
	= std::common_comparison_category_t<std::__detail::__synth3way_t<Ts, Us>...>;
      return std::__tuple_cmp<_Cat>(*this, other, std::make_index_sequence<sizeof...(Ts)>{});

            // auto result = std::strong_ordering::equivalent;
            // const auto compare = [&](const auto & lhs, const auto & rhs){
            //     if (result != std::strong_ordering::equivalent) return;
            //     result = (lhs <=> rhs);
            // };
            // [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            //     ((compare(get<indexes>(), other.template get<indexes>())), ...);
            // }(std::make_index_sequence<sizeof...(Ts)>{});
            // return result;
        }
        
        #if false
          template<typename... _TElements, typename... _UElements>
            constexpr bool
            operator<(const tuple<_TElements...>& __t,
                const tuple<_UElements...>& __u)
            {
            static_assert(sizeof...(_TElements) == sizeof...(_UElements),
            "tuple objects can only be compared if they have equal sizes.");
            using __compare = __tuple_compare<tuple<_TElements...>,
                            tuple<_UElements...>,
                            0, sizeof...(_TElements)>;
            return __compare::__less(__t, __u);
            }

        template<typename... _TElements, typename... _UElements>
            constexpr bool
            operator!=(const tuple<_TElements...>& __t,
                const tuple<_UElements...>& __u)
            { return !(__t == __u); }

        template<typename... _TElements, typename... _UElements>
            constexpr bool
            operator>(const tuple<_TElements...>& __t,
                const tuple<_UElements...>& __u)
            { return __u < __t; }

        template<typename... _TElements, typename... _UElements>
            constexpr bool
            operator<=(const tuple<_TElements...>& __t,
                const tuple<_UElements...>& __u)
            { return !(__u < __t); }

        template<typename... _TElements, typename... _UElements>
            constexpr bool
            operator>=(const tuple<_TElements...>& __t,
                const tuple<_UElements...>& __u)
            { return !(__t < __u); }
        #endif


        // TODO(Guillaume): if C++23, use deducing this, rather than such a quadruplication
        template <std::size_t index>
        [[nodiscard]] constexpr auto & get() & noexcept {
            static_assert(index <= size, "csl::mp::tuple::get<size_t>: out-of-bounds");
            using accessor = details::tuple_storage_accessor_t<typename type::template nth_<index>>;
            return static_cast<accessor&>(storage).value;
        }
        template <std::size_t index>
        [[nodiscard]] constexpr const auto & get() const & noexcept {
            static_assert(index <= size, "csl::mp::tuple::get<size_t>: out-of-bounds");
            using accessor = details::tuple_storage_accessor_t<typename type::template nth_<index>>;
            return static_cast<const accessor&>(storage).value;
        }
        template <std::size_t index>
        [[nodiscard]] constexpr auto && get() && noexcept {
            static_assert(index <= size, "csl::mp::tuple::get<size_t>: out-of-bounds");
            using accessor = details::tuple_storage_accessor_t<typename type::template nth_<index>>;
            return static_cast<accessor &&>(std::move(storage)).value;
        }
        template <std::size_t index>
        [[nodiscard]] constexpr const auto && get() const && noexcept {
            static_assert(index <= size, "csl::mp::tuple::get<size_t>: out-of-bounds");
            using accessor = details::tuple_storage_accessor_t<typename type::template nth_<index>>;
            return static_cast<const accessor &&>(std::move(storage)).value;
        }

    // storage accessors
        // get/at/operator[] -> cvref qualifiers matrix
        // assign/operator=(tuple<Us...>) if (true and ... and std::assignable_to<Ts, Us>)
        // compare/operator<=>

        // visit/operator()

    private:
        storage_type storage;
    };
    template <typename ... Ts>
    tuple(Ts && ...) -> tuple<std::remove_cvref_t<Ts>...>;

    // compare
    // template <typename ... Ts, typename ... Us>
    // requires (sizeof...(Ts) == sizeof...(Us))
    //      and (true and ... and std::three_way_comparable_with<Ts, Us>)
    // constexpr inline static auto operator<=>(
    //     const tuple<Ts...> & lhs,
    //     const tuple<Us...> & rhs
    // ){
    //     return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //         return ((get<indexes>(lhs) <=> get<indexes>(rhs)) <=> ...);
    //     }(std::make_index_sequence<sizeof...(Ts)>{});
    // }

    // is_tuple
    template <typename>
    struct is_tuple : std::false_type{};
    template <typename ... Ts>
    struct is_tuple<tuple<Ts...>> : std::true_type{};
    template <typename T>
    constexpr bool is_tuple_v = is_tuple<T>::value;

    namespace concepts {
        template <typename T> concept tuple = is_tuple_v<std::remove_cvref_t<T>>;
    }
}
namespace csl::mp::details::concepts {
    // deductible: type
    template <typename tuple_type, typename T>
    concept can_deduce_by_type = is_tuple_v<tuple_type>
        and requires { std::void_t<typename tuple_type::template by_type_<T>>(); }
    ;
    // deductible: type element
    template <typename tuple_type, std::size_t I>
    concept can_deduce_by_index = is_tuple_v<tuple_type>
        and requires { std::void_t<typename tuple_type::template nth_<I>>(); }
    ;
}
namespace csl::mp {

    // size
    template <typename>
    struct tuple_size;
    template <typename ... Ts>
    struct tuple_size<tuple<Ts...>> : std::integral_constant<std::size_t,
        sizeof...(Ts)
    >{};
    template <typename tuple_type>
    constexpr std::size_t tuple_size_v = tuple_size<tuple_type>::value;

    // type-by-index
    template <std::size_t, typename>
    struct tuple_element;
    template <std::size_t index, typename ... Ts>
    struct tuple_element<index, tuple<Ts...>> : tuple<Ts...>::template nth_<index>{};
    template <std::size_t index, typename tuple_type>
    using tuple_element_t = typename tuple_element<index, tuple_type>::type;

    template <std::size_t index, typename T>
    using nth = tuple_element_t<index, T>;

    template <typename T>
    struct is_valid_tuple : std::false_type{};
    template <typename ... Ts>
    struct is_valid_tuple<tuple<Ts...>> : std::bool_constant<
    // no duplicates types
        (true and ... and details::concepts::can_deduce_by_type<tuple<Ts...>, Ts>)
    >{};
    template <typename T>
    constexpr bool is_valid_tuple_v = is_valid_tuple<T>::value;

    namespace concepts {
        template <typename T> concept valid_tuple = is_valid_tuple_v<T>;
    }

    // empty
    template <typename>
    struct empty;
    template <>
    struct empty<tuple<>> : std::true_type{};
    template <typename T0, typename ... Ts>
    struct empty<tuple<T0, Ts...>> : std::false_type{};
    template <typename T>
    constexpr bool empty_v = empty<T>::value;

    namespace concepts {
        template <typename T>
        concept empty_tuple = tuple<T> and empty_v<T>;
    }

    // count
    template <typename, typename>
    struct count;
    template <typename T, typename ... Ts>
    struct count<T, tuple<Ts...>> : std::integral_constant<std::size_t,
        (0 + ... + std::is_same_v<T, Ts>)
    >{};
    template <typename T, typename tuple_type>
    constexpr std::size_t count_v = count<T, tuple_type>::value;

    // count_if
    template <template <typename...> typename, typename>
    struct count_if;
    template <template <typename...> typename trait, typename ... Ts>
    struct count_if<trait, csl::mp::tuple<Ts...>> : std::integral_constant<std::size_t,
        (0 + ... + trait<Ts>::value)
    >{};
    template <template <typename...> typename trait, typename tuple_type>
    constexpr std::size_t count_if_v = count_if<trait, tuple_type>::value;

    // index-by-type
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct index_of : std::integral_constant<std::size_t,
        tuple_type::template by_type_<T>::index
    >{};
    template <typename T, concepts::valid_tuple tuple_type>
    constexpr std::size_t index_of_v = index_of<T, tuple_type>::value;

    // unfold_into
    template <template <typename...> typename, typename>
    struct unfold_into;
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_into<destination, tuple<Ts...>> : type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename tuple_type>
    using unfold_into_t = typename unfold_into<destination, tuple_type>::type;

    // transform
    template <template <typename> typename, typename>
    struct transform;
    template <template <typename> typename trait, typename ... Ts>
    struct transform<trait, csl::mp::tuple<Ts...>> : type_identity<
        typename csl::mp::tuple<typename trait<Ts>::type...>
    >{};
    template <template <typename> typename trait, typename tuple_type>
    using transform_t = typename transform<trait, tuple_type>::type;

    // tuple_cat
    constexpr auto tuple_cat(){ return csl::mp::tuple{}; }
    constexpr auto tuple_cat(/* TODO: tuplelike */ auto && ... tuples)
    requires (sizeof...(tuples) not_eq 0)
    {
        constexpr auto size = (... + csl::mp::tuple_size_v<std::remove_cvref_t<decltype(tuples)>>);

        if constexpr (size == 0)
            return csl::mp::tuple<>{};
        else
        {
            // TODO(Guillaume): use `make_two_dimension_index` from the old `flatten` poc ?
            constexpr auto indexes_map = [&](){

                // scenario:
                // t0 = [ a b . . . ]
                // t1 = [ . . c d . ]
                // t2 = [ . . . . e ]

                //  tuple_I    || element_I
                // --------------------------
                // [ . . . . . ][ . . . . . ]
                // --------------------------
                // [ 0 . . . . ][ 0 . . . . ]
                // [ 0 0 . . . ][ 0 1 . . . ]
                // --------------------------
                // [ 0 0 1 . . ][ 0 1 0 . . ]
                // [ 0 0 1 1 . ][ 0 1 0 1 . ]
                // --------------------------
                // [ 0 0 1 1 2 ][ 0 1 0 1 0 ]

                struct {
                    std::size_t tuple_index[size];      // NOLINT(*-avoid-c-arrays)
                    std::size_t element_index[size];    // NOLINT(*-avoid-c-arrays)
                } mapped_indexes{};

                auto create_indexes_for = [
                    &,
                    tuple_index = std::size_t{0},
                    offset = std::size_t{0}
                ]<std::size_t ... indexes>(std::index_sequence<indexes...>) mutable {

                    ((
                        (mapped_indexes.tuple_index  [offset + indexes] = tuple_index),
                        (mapped_indexes.element_index[offset + indexes] = indexes)
                    ), ...);

                    offset += sizeof...(indexes);
                    ++tuple_index;
                };

                // create indexes for each tuple
                ((create_indexes_for(std::make_index_sequence<
                    csl::mp::tuple_size_v<std::remove_cvref_t<decltype(tuples)>>
                >{})), ...);

                return mapped_indexes;
            }();

            return []<std::size_t ... indexes>(auto && tuples, std::index_sequence<indexes...>){

                using tuples_type = std::remove_cvref_t<decltype(tuples)>;
                using type = csl::mp::tuple<
                    csl::mp::tuple_element_t<
                        indexes_map.element_index[indexes],
                        std::remove_cvref_t<csl::mp::tuple_element_t<
                            indexes_map.tuple_index[indexes],
                            tuples_type
                        >>
                    >...
                >;
                return type{
                    get<indexes_map.element_index[indexes]>(
                        get<indexes_map.tuple_index[indexes]>(fwd(tuples))
                    )...
                };
            }(
                csl::mp::tuple<std::remove_cvref_t<decltype(tuples)>...>{ fwd(tuples)... },
                std::make_index_sequence<size>{}
            );
        }
    }

    // tuple_cat_result
    template <typename ... tuple_types>
    struct tuple_cat_result : csl::mp::type_identity<
        decltype(tuple_cat(std::declval<tuple_types>()...))
    >{};
    template <typename ... tuple_types>
    using tuple_cat_result_t = typename tuple_cat_result<tuple_types...>::type;

    // contains
    // benchmark (~ +10% perfs): https://build-bench.com/b/uZluytW6RILwmbT8SpMrpfiINoA
    template <typename, typename>
    struct contains;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct contains<T, tuple_type> : std::true_type{};
    template <typename T, typename ... Ts>
    struct contains<T, tuple<Ts...>> : std::bool_constant<
        (std::same_as<T, Ts> or ...)
    >{};
    template <typename T, typename tuple_type>
    constexpr bool contains_v = contains<T, tuple_type>::value;

    // has_duplicates
    template <typename>
    struct has_duplicates;
    template <typename ... Ts>
    struct has_duplicates<tuple<Ts...>> : std::negation<is_valid_tuple<tuple<Ts...>>>{};
    template <typename tuple_type>
    constexpr bool has_duplicates_v = has_duplicates<tuple_type>::value;

    // first_index_of
    template <typename, typename>
    struct first_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct first_index_of<T, tuple_type> : index_of<T, tuple_type>{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct first_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            (void)((pos = (pos == std::string::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "first_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t first_index_of_v = first_index_of<T, tuple_type>::value;

    // rfirst_index_of
    template <typename, typename>
    struct rfirst_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct rfirst_index_of<T, tuple_type> : std::bool_constant<
        tuple_size_v<tuple_type> - 1 - index_of_v<T, tuple_type>
    >{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct rfirst_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            (void)((pos = (pos == std::string::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(csl::mp::seq::make_reverse_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "rfirst_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t rfirst_index_of_v = rfirst_index_of<T, tuple_type>::value;

    // last_index_of
    //  equivalent to (sizeof...(Ts) - rfirst_index_of<T, tuple<Ts...>>)
    template <typename, typename>
    struct last_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct last_index_of<T, tuple_type> : index_of<T, tuple_type>{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct last_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            (void)((pos = std::is_same_v<T, Ts> ? indexes : pos), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "last_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t last_index_of_v = last_index_of<T, tuple_type>::value;

    // set_union
    template <typename, typename>
    struct set_union;
    template <typename ... Ts, typename ... Us>
    struct set_union<tuple<Ts...>, tuple<Us...>> : std::type_identity<
        decltype(tuple_cat(
            tuple<Ts...>{},
            std::conditional_t<
                contains_v<Us, tuple<Ts...>>,
                tuple<>,
                tuple<Us>
            >{}...
        ))
    >{};
    template <typename T, typename U>
    using set_union_t = typename set_union<T, U>::type;

    // set_intersection
    template <typename, typename>
    struct set_intersection;
    template <typename ... Ts, typename ... Us>
    struct set_intersection<tuple<Ts...>, tuple<Us...>> : std::type_identity<
        decltype(tuple_cat(
            std::conditional_t<
                contains_v<Us, tuple<Ts...>>,
                tuple<Us>,
                tuple<>
            >{}...
        ))
    >{};
    template <typename T, typename U>
    using set_intersection_t = typename set_intersection<T, U>::type;

    // is_unique
    template <typename, typename>
    struct is_unique;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct is_unique<T, tuple_type> : std::true_type{};
    template <typename T, typename ... Ts>
    requires (not contains_v<T, tuple<Ts...>>)
    struct is_unique<T, tuple<Ts...>> : std::false_type{};
    template <typename T, typename ... Ts>
    struct is_unique<T, tuple<Ts...>> : std::bool_constant<
        (first_index_of_v<T, tuple<Ts...>> == last_index_of_v<T, tuple<Ts...>>)
    >{};
    template <typename T, typename tuple_type>
    constexpr bool is_unique_v = is_unique<T, tuple_type>::value;

    // filter<trait>
    template <typename, template <typename...> typename>
    struct filter;
    template <typename ... Ts, template <typename...> typename predicate>
    struct filter<tuple<Ts...>, predicate> : tuple_cat_result<
        std::conditional_t<
            predicate<Ts>::value,
            tuple<Ts>, tuple<>
        >...
    >{};
    template <concepts::tuple tuple_type, template <typename...> typename predicate>
    using filter_t = typename filter<tuple_type, predicate>::type;

    // deduplicate / make_valid
    // TODO(Guss): reverse prior to filtering, to preserve order
    template <typename>
    struct deduplicate;
    template <concepts::valid_tuple T>
    struct deduplicate<T> : type_identity<T>{};
    template <typename ... Ts>
    struct deduplicate<tuple<Ts...>> : 
    //  // equivalent to filter<tuple<Ts...>, bind_back<is_unique, tuple<Ts...>>::type>{};
    //  tuple_cat_result<
    //     std::conditional_t<
    //         is_unique_v<Ts, tuple<Ts...>>,
    //         tuple<Ts>, tuple<>
    //     >...
    // >{};
    decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        return tuple_cat_result<
            std::conditional_t<
                details::concepts::can_deduce_by_type<tuple<Ts...>, Ts>
                or indexes == first_index_of_v<Ts, tuple<Ts...>>,
                tuple<Ts>, tuple<>
            >...
        >{};
    }(std::make_index_sequence<tuple_size_v<tuple<Ts...>>>{})){};
    template <concepts::tuple tuple_type>
    using deduplicate_t = typename deduplicate<tuple_type>::type;

    // flatten_once
    // flatten / make_flat

    // fwd_as_tuple
    // get<I>, get<T>
    // // for_each
    // template <concepts::valid_tuple tuple_type>
    // void for_each(tuple_type && value, auto && visitor){
    //     [&](){}();
    // }

    // for_each_with_index
    // template <concepts::valid_tuple tuple_type>
    // void for_each_with_index(tuple_type && value, auto && visitor){
    //     [&](){}();
    // }

    // apply

    // projection: column vs. row
    // column: projection by index -> tuple{ tuple{ int, char }, tuple{ int, char } }
    //  project_by_index<0>(tuple_of_tuplelike) -> tuple{ int, int }
    //  project_by_index<1>(tuple_of_tuplelike) -> tuple{ char, char }

    // tuple_storage: if are_same<Ts...> -> use std::array instead

    // print:
    //  fmt (opt-in or detected dependencies)
    //      -> csl::ag-like compact vs. pretty printing
    //  std::ostream printing
}

// tuple: API
namespace csl::mp {

    // tuple_size
    // tuple_element

    template <std::size_t index>
    [[nodiscard]] constexpr auto get(concepts::tuple auto && value) noexcept -> decltype(auto) {
        return value.template get<index>();
    }
    // WIP

    // make_tuple
    // forward_as_tuple
}

#pragma region std utility inter-operatiblity (structured binding)

//  N4606 [namespace.std]/1 :
//  A program may add a template specialization for any standard library template to namespace std
//  only if the declaration depends on a user-defined type 
//  and the specialization meets the standard library requirements for the original template and is not explicitly prohibited.

#include <utility> // std::tuple_size, std::tuple_element

// NOLINTBEGIN(cert-dcl58-cpp) Modification of 'std' namespace can result in undefined behavior
template <csl::mp::concepts::tuple tuple_type>
struct std::tuple_size<tuple_type> : csl::mp::tuple_size<tuple_type>{};

template <std::size_t index, csl::mp::concepts::tuple tuple_type>
struct std::tuple_element<index, tuple_type> : csl::mp::tuple_element<index, tuple_type>{};
// NOLINTEND(cert-dcl58-cpp)
#pragma endregion


// TODO(@Guss): algos eDSL (range-like?) : (pipe, shift operators, plus, minus, etc...)


// ===================
// -- OLD, to refactor
// ===================

#if false

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

    // unfold_into
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_into : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_into<destination, from<Ts...>> : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_into_t = typename unfold_into<destination, from...>::type;

    // flat_cat
    template <typename, typename ...>
    struct flat_cat;
    template <template <typename...> typename pack_type, typename ... Ts, typename ... rest>
    struct flat_cat<pack_type<Ts...>, rest...>
    : std::type_identity<
        unfold_into_t<pack_type, decltype(std::tuple_cat(
            std::tuple<Ts...>{},
            std::conditional_t<
                is_instance_of_v<pack_type, rest>,
                unfold_into_t<std::tuple, rest>,
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
        unfold_into_t<pack_type, decltype(std::tuple_cat(
            std::conditional_t<
                is_instance_of_v<pack_type, Ts>,
                unfold_into_t<std::tuple, Ts>,
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
                // todo : reverse, type_at
            }();
        }
    };
    template <typename T>
    using reverse_t = typename reverse<T>::type;

#ifdef false // TODO: refactor
    // rindex_of
    template <typename T, typename ... Ts>
    struct rindex_of {
        constexpr static std::size_t value = rindex_of<T, unfold_into_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename...> typename pack, typename ... Ts>
    struct rindex_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            ((pos = (std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq std::string::npos, "rindex_of : not found");
    };
    template <typename T, typename ... Ts>
    constexpr std::size_t rindex_of_v = rindex_of<T, Ts...>::value;

    // index_of
    template <typename T, typename ... Ts>
    struct index_of {
        constexpr static std::size_t value = index_of<T, unfold_into_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename ...> typename pack, typename ... Ts>
    struct index_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = std::string::npos;
            ((pos = (pos == std::string::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
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
    using last_index_of = rindex_of<T, Ts...>;
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
        using type = unfold_into_t<type_pack, tuple_type>;
    };
    template <typename T>
    using deduplicate_t = typename deduplicate<T>::type;
}

#endif

#undef fwd
#undef static_dependent_error

// wip : https://godbolt.org/z/TfMqM5TaG
// wip, benchmarked : https://godbolt.org/z/fj4z8sjzh
// wip bench : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
