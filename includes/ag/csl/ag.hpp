#pragma once

#if not __cplusplus >= 202002L
# error "csl/ag.hpp requires C++20"
#endif

#include <array>
#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>
#include <climits>
#include <string_view>
#include <algorithm>
#include <stdexcept>

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ag::details::unevaluated {
// for unevaluated context only

    template <typename T>
    consteval auto declval() noexcept -> std::add_rvalue_reference_t<T>  {
        // static_assert([](){ return false; }(), "csl::ag::details : declval : for unevaluated context only !");
        if constexpr (std::is_lvalue_reference_v<T>)
            return *((std::remove_reference_t<T>*){ nullptr });
        else
            return std::move(*((std::remove_reference_t<T>*){ nullptr }));
    }

    struct ref_evaluator {
        explicit constexpr ref_evaluator() = delete;
        constexpr ~ref_evaluator() = delete;
        constexpr ref_evaluator(const ref_evaluator&) = delete;
        constexpr ref_evaluator(ref_evaluator&&) = delete;
        constexpr ref_evaluator & operator=(const ref_evaluator&) = delete;
        constexpr ref_evaluator & operator=(ref_evaluator&&) = delete;

		// Implicit conversion
        // 	not `return std::declval<T>();`, as clang does not like it even in a non-evaluated context
        // 	neither `consteval` -> Clang-16.0.? ICE
        template <typename T>
        [[nodiscard]] consteval operator T&&() const noexcept { // NOLINT(google-explicit-constructor)
            return declval<T&&>();
        }
        template <typename T>
        [[nodiscard]] consteval operator T&() const noexcept { // NOLINT(google-explicit-constructor)
            return declval<T&>();
        }
    };
}
namespace csl::ag::details::mp {

    // NTTP-dependent type
    template <typename T, auto index>
    struct unfolder : std::type_identity<T>{};
    template <typename T, auto index>
    using unfolder_t = typename unfolder<T, index>::type;

// P1450 Enriching type modification traits : https://github.com/cplusplus/papers/issues/216
// Note that this is a good-enough implementation of P1450 to only fit this project's needs

    // P1450 copy_ref
    template <typename from, typename to>
    struct copy_ref : std::remove_reference<to>{};
    template <typename from, typename to>
    struct copy_ref<from&, to> : std::add_lvalue_reference<to>{};
    template <typename from, typename to>
    struct copy_ref<from&&, to> : std::add_rvalue_reference<std::remove_reference_t<to>>{};
    template <typename from, typename to>
    using copy_ref_t = typename copy_ref<from, to>::type;

    // P1450 - add cv - impl detail (also for ref-qualified types)
    template <typename T> struct add_const : std::type_identity<const T>{};
    template <typename T> struct add_const<T&> : std::type_identity<const T&>{};
    template <typename T> struct add_const<T&&> : std::type_identity<const T&&>{};
    template <typename T> using add_const_t = typename add_const<T>::type;

    template <typename T> struct add_volatile : std::type_identity<volatile T>{};
    template <typename T> struct add_volatile<T&> : std::type_identity<volatile T&>{};
    template <typename T> struct add_volatile<T&&> : std::type_identity<volatile T&&>{};
    template <typename T> using add_volatile_t  = typename add_volatile<T>::type;

    template <typename T> struct add_cv : add_const<typename add_volatile<T>::type>{};
    template <typename T> using add_cv_t = typename add_cv<T>::type;

    // P1450 copy_cv
    template <typename from, typename to>
    struct copy_cv : std::remove_cv<to>{};
    template <typename from, typename to> requires (std::is_reference_v<from>)
    struct copy_cv<from, to> : copy_cv<std::remove_reference_t<from>, to>{};
    template <typename from, typename to>
    struct copy_cv<const volatile from, to> : add_cv<to>{};
    template <typename from, typename to>
    struct copy_cv<const from, to> : add_const<to>{};
    template <typename from, typename to>
    struct copy_cv<volatile from, to> : add_volatile<to>{};
    template <typename from, typename to>
    using copy_cv_t = typename copy_cv<from, to>::type;

    // P1450 copy_cvref
    template <typename from, typename to>
    struct copy_cvref : copy_cv<from, copy_ref_t<from, to>>{};
    template <typename from, typename to>
    using copy_cvref_t = typename copy_cvref<from, to>::type;

    // field_view
    template <typename owner, typename T>
    struct field_view : copy_cvref<owner, T>{};
    template <typename owner, typename T>
    requires (std::is_reference_v<T>)
    struct field_view<owner, T> : std::type_identity<T>{};
    template <typename owner, typename T>
    using field_view_t = typename field_view<owner, T>::type;

    // bind_front
    // TODO: remove ?
    template <template <typename ...> typename trait, typename ... bound_Ts>
    struct bind_front {
        template <typename ... Ts>
        using type = typename trait<bound_Ts..., Ts...>::type;
        template <typename ... Ts>
        constexpr inline static auto value = trait<bound_Ts..., Ts...>::value;
    };

    template <class, class>
    struct first_index_of;
    template <class T, typename... Ts>
    struct first_index_of<T, std::tuple<Ts...>> : std::integral_constant<std::size_t, 
        [](){
            static_assert(sizeof...(Ts));
            constexpr auto results = std::array{ std::is_same_v<T, Ts>... };
            const auto it = std::find(std::cbegin(results), std::cend(results), true);
            if (it == std::cend(results))
                throw std::runtime_error{"csl::ag::details::mp:first_index_of<T, tuple_type>: no match"};
            return std::distance(std::cbegin(results), it);
        }()
    >{};
    template <class T, class tuple_type>
    constexpr inline static auto first_index_of_v = first_index_of<T, tuple_type>::value;
}
namespace csl::ag::concepts {

	template <typename T>
	concept unqualified_aggregate =
        std::is_aggregate_v<T> and
		not std::is_empty_v<T> and
		not std::is_union_v<T> and
		not std::is_polymorphic_v<T> and
		not std::is_reference_v<T>
	;
    template <typename T>
    concept aggregate = unqualified_aggregate<std::remove_cvref_t<T>>
    ;
    template <typename T>
    concept aggregate_default_initializable = aggregate<T> and std::default_initializable<T>;

    template <typename T, typename... args_ts>
    concept aggregate_constructible_from = unqualified_aggregate<T> and requires { T{ std::declval<args_ts>()... }; }
    ;
    template <typename T, std::size_t size>
    concept aggregate_constructible_from_n_values =
        concepts::aggregate<T> and
        []<std::size_t... indexes>(std::index_sequence<indexes...>) {
            return concepts::aggregate_constructible_from<
                T,
                details::mp::unfolder_t<details::unevaluated::ref_evaluator, indexes>...
            >;
        }(std::make_index_sequence<size>{})
    ;

    // P2165 - tuple-like
    // Note that this is a good-enough implementation of P2165 to only fit this project's needs
	template <typename T, std::size_t N>
    concept is_tuple_element = requires(T t) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T>&>;
    };
    template <typename T>
    concept tuple_like =
        not std::is_reference_v<T>
        and requires {
            typename std::tuple_size<T>::type;
            requires std::same_as<std::remove_const_t<decltype(std::tuple_size_v<T>)>, std::size_t>;
        }
        and []<std::size_t... I>(std::index_sequence<I...>) {
            return (is_tuple_element<T, I> && ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>{})
    ;
    template <typename T>
    concept pair_like = tuple_like<T> and std::tuple_size_v<T> == 2;

	template <typename T>
	concept structured_bindable = tuple_like<T> or aggregate<T>;
}
namespace csl::ag::details {

#pragma region fields_count
    #if not defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
    # if defined(CSL_AG__VERBOSE_BUILD)
    #  pragma message("csl::ag : CSL_AG__ENABLE_BITFIELDS_SUPPORT [disabled], faster algorithm selected")
    # endif
	template <concepts::aggregate T, std::size_t indice>
    requires (std::default_initializable<T>)
    [[nodiscard]] consteval auto fields_count_impl() noexcept -> std::size_t {
    // faster algorithm if T is default_initializable (ref fields can be initialized),
    // and no fields is a bitfield.
        static_assert(not std::is_reference_v<T>, "concepts::aggregate T cannot be cv-ref qualified");

        if constexpr (indice == 0) {
            static_assert(indice != 0, "csl::ag::details::fields_count (w/o ref) : Cannot evalute T's field count");
            return {}; // no-return
        }

        if constexpr (concepts::aggregate_constructible_from_n_values<T, indice>)
            return indice;
        else if constexpr (not concepts::aggregate_constructible_from_n_values<T, indice / 2 + 1>)
            return fields_count_impl<T, indice / 2>();
        else
            return fields_count_impl<T, indice - 1>();
    }
    #else
    # pragma message("csl::ag : CSL_AG__ENABLE_BITFIELDS_SUPPORT [enabled], slower algorithm selected")
    #endif

    template <concepts::aggregate T, std::size_t indice>
    [[nodiscard]] consteval auto fields_count_impl() noexcept -> std::size_t {
    // costly algorithm
        static_assert(not std::is_reference_v<T>);

        if constexpr (indice == 0) {
            static_assert(indice != 0, "csl::ag::details::fields_count (with ref) : Cannot evalute T's field count");
            return {}; // no-return
        }

        if constexpr (concepts::aggregate_constructible_from_n_values<T, indice>)
            return indice;
        else
            return fields_count_impl<T, indice - 1>();
    }

	template <concepts::aggregate T>
    constexpr inline static std::size_t fields_count = fields_count_impl<
        T,
        sizeof(T)
        #if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
        * sizeof(std::byte) * CHAR_BIT
        #endif
    >();
#pragma endregion
#pragma region to_tuple
	// generated : interface
    namespace generated {
        template <std::size_t N>
        [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto &&) noexcept
		// -> std::type_identity<std::tuple<csl::ag::element<I, value_t>...>>
        {
            static_assert([](){ return false; }(), "[csl] exceed maxmimum members count");
        }
        template <std::size_t>
        [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto &&) noexcept
		// -> std::tuple<decltype(get<I>(value))...>
        {
            static_assert([](){ return false; }(), "[csl] exceed maxmimum members count");
        }
    }

    consteval auto make_to_tuple(concepts::aggregate auto && value) /* -> std::type_identity<std::tuple<field_Ts...>>*/;
    template <typename T>
    using to_tuple_t = mp::copy_cvref_t<
        T,
        typename std::remove_cvref_t<decltype(csl::ag::details::make_to_tuple(std::declval<std::remove_cvref_t<T>>()))>::type
    >;
#pragma endregion

    template <typename owner_type>
    [[nodiscard]] constexpr inline static concepts::tuple_like auto make_tuple_view(auto && ... values) noexcept {
        using tuple_t = to_tuple_t<std::remove_cvref_t<owner_type>>;
        
        constexpr auto size = std::tuple_size_v<tuple_t>;
        static_assert(size == sizeof...(values));
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::forward_as_tuple(
                static_cast<mp::field_view_t<owner_type, std::tuple_element_t<indexes, tuple_t>>>(values)...
            );
        }(std::make_index_sequence<size>{});
    }
}
// --- generated: details ---
namespace csl::ag::details::generated {
// GENERATED CONTENT, DO NOT EDIT MANUALLY !
// Generated code with CSL_AG_MAX_FIELDS_COUNT_OPTION = 32
#pragma region make_to_tuple<N,T>
template <std::size_t N> requires (N == 1) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0 ] = value;
	return std::type_identity<std::tuple<decltype(v0)>>{};
}
template <std::size_t N> requires (N == 2) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1)>>{};
}
template <std::size_t N> requires (N == 3) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2)>>{};
}
template <std::size_t N> requires (N == 4) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3)>>{};
}
template <std::size_t N> requires (N == 5) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4)>>{};
}
template <std::size_t N> requires (N == 6) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5)>>{};
}
template <std::size_t N> requires (N == 7) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6)>>{};
}
template <std::size_t N> requires (N == 8) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7)>>{};
}
template <std::size_t N> requires (N == 9) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8)>>{};
}
template <std::size_t N> requires (N == 10) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9)>>{};
}
template <std::size_t N> requires (N == 11) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10)>>{};
}
template <std::size_t N> requires (N == 12) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11)>>{};
}
template <std::size_t N> requires (N == 13) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12)>>{};
}
template <std::size_t N> requires (N == 14) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13)>>{};
}
template <std::size_t N> requires (N == 15) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14)>>{};
}
template <std::size_t N> requires (N == 16) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15)>>{};
}
template <std::size_t N> requires (N == 17) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16)>>{};
}
template <std::size_t N> requires (N == 18) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17)>>{};
}
template <std::size_t N> requires (N == 19) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18)>>{};
}
template <std::size_t N> requires (N == 20) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19)>>{};
}
template <std::size_t N> requires (N == 21) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20)>>{};
}
template <std::size_t N> requires (N == 22) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21)>>{};
}
template <std::size_t N> requires (N == 23) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22)>>{};
}
template <std::size_t N> requires (N == 24) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23)>>{};
}
template <std::size_t N> requires (N == 25) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24)>>{};
}
template <std::size_t N> requires (N == 26) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25)>>{};
}
template <std::size_t N> requires (N == 27) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26)>>{};
}
template <std::size_t N> requires (N == 28) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27)>>{};
}
template <std::size_t N> requires (N == 29) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28)>>{};
}
template <std::size_t N> requires (N == 30) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29)>>{};
}
template <std::size_t N> requires (N == 31) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30)>>{};
}
template <std::size_t N> requires (N == 32) // NOLINT
 [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = value;
	return std::type_identity<std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31)>>{};
}
#pragma endregion
#pragma region to_tuple_view_impl<N,T>
template <std::size_t N> requires (N == 1) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0) );
}
template <std::size_t N> requires (N == 2) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1) );
}
template <std::size_t N> requires (N == 3) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2) );
}
template <std::size_t N> requires (N == 4) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3) );
}
template <std::size_t N> requires (N == 5) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4) );
}
template <std::size_t N> requires (N == 6) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5) );
}
template <std::size_t N> requires (N == 7) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6) );
}
template <std::size_t N> requires (N == 8) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7) );
}
template <std::size_t N> requires (N == 9) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8) );
}
template <std::size_t N> requires (N == 10) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9) );
}
template <std::size_t N> requires (N == 11) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10) );
}
template <std::size_t N> requires (N == 12) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11) );
}
template <std::size_t N> requires (N == 13) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12) );
}
template <std::size_t N> requires (N == 14) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13) );
}
template <std::size_t N> requires (N == 15) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14) );
}
template <std::size_t N> requires (N == 16) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15) );
}
template <std::size_t N> requires (N == 17) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16) );
}
template <std::size_t N> requires (N == 18) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17) );
}
template <std::size_t N> requires (N == 19) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18) );
}
template <std::size_t N> requires (N == 20) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19) );
}
template <std::size_t N> requires (N == 21) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20) );
}
template <std::size_t N> requires (N == 22) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21) );
}
template <std::size_t N> requires (N == 23) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22) );
}
template <std::size_t N> requires (N == 24) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23) );
}
template <std::size_t N> requires (N == 25) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24) );
}
template <std::size_t N> requires (N == 26) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25) );
}
template <std::size_t N> requires (N == 27) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26) );
}
template <std::size_t N> requires (N == 28) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27) );
}
template <std::size_t N> requires (N == 29) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28) );
}
template <std::size_t N> requires (N == 30) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29) );
}
template <std::size_t N> requires (N == 31) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30) );
}
template <std::size_t N> requires (N == 32) // NOLINT
 [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = value;
	return make_tuple_view<decltype(value)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30),csl_fwd(v31) );
}
#pragma endregion
// END OF GENERATED CONTENT
}
namespace csl::ag::details {
    [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value)
    // -> std::type_identity<std::tuple<field_Ts...>>
    {
        constexpr auto size = fields_count<std::remove_cvref_t<decltype(value)>>;
        return details::generated::make_to_tuple<size>(csl_fwd(value));
    }
}
// --- API ---
namespace csl::ag {

    // to_tuple
    template <concepts::aggregate T>
    using to_tuple_t = details::to_tuple_t<T>;

    // --- inner API ---
    // size
    template <csl::ag::concepts::aggregate T>
    struct size : std::integral_constant<std::size_t, details::fields_count<std::remove_reference_t<T>>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr inline static auto size_v = size<T>::value;

    template <csl::ag::concepts::aggregate T>
    struct empty: std::bool_constant<(size<T>::value == 0)>{};
    template <csl::ag::concepts::aggregate T>
    constexpr inline static auto empty_v = empty<T>::value;

    // element
	template <std::size_t N, concepts::aggregate T>
    using element = std::tuple_element<N, details::to_tuple_t<std::remove_cvref_t<T>>>;
	template <std::size_t N, concepts::aggregate T>
	using element_t = typename element<N, T>::type;

    // tuple-view
    //  factory that creates a lightweight accessor to an existing aggregate value,
    //  extending owner's value-semantic to owned values,
    //  while preserving value-semantic of ref-qualified values
    //  ex:
    //  - struct type{ A v0; B & v1; const C && v2 }
    //  -       type &  => std::tuple<      A&,  B&, const C&&>;
    //  - const type &  => std::tuple<const A&,  B&, const C&&>;
    //  -       type && => std::tuple<      A&&, B&, const C&&>;
    [[nodiscard]] constexpr auto to_tuple_view(concepts::aggregate auto && value) noexcept {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::generated::to_tuple_view_impl<details::fields_count<type>>(std::forward<decltype(value)>(value));
    }
    // TODO(Guss): view -> tuple + is_product;
    //  - is_view
    //  - is_owning -> not_ref<Ts> and ...
    //  - is_non_owning -> ref<Ts> and ...
    template <concepts::aggregate T> requires (std::is_reference_v<T>)
    struct view : std::type_identity<decltype(to_tuple_view(std::declval<T>()))>{}; 
    template <concepts::aggregate T> requires (std::is_reference_v<T>)
    using view_t = typename view<T>::type;

    // view_element
	template <std::size_t N, concepts::aggregate T>
    requires (std::is_reference_v<T>)
    struct view_element : std::tuple_element<N, view_t<T>>{};
	template <std::size_t N, concepts::aggregate T>
	using view_element_t = typename view_element<N, T>::type;

    // --- tuple-like ---
    // tuple_size
    template <csl::ag::concepts::aggregate T>
    struct tuple_size : std::integral_constant<std::size_t, details::fields_count<std::remove_reference_t<T>>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr inline static auto tuple_size_v = tuple_size<T>::value;

    // tuple_element
    template <std::size_t N, concepts::aggregate T>
    using tuple_element = std::tuple_element<N, details::to_tuple_t<std::remove_cvref_t<T>>>;
	template <std::size_t N, concepts::aggregate T>
	using tuple_element_t = typename tuple_element<N, T>::type;

    // get<std::size_t>
    template <std::size_t N>
    [[nodiscard]] constexpr decltype(auto) get(concepts::aggregate auto && value) noexcept {
        static_assert(N < size_v<std::remove_cvref_t<decltype(value)>>, "csl::ag::get<std::size_t>: index >= size_v");
        return ::std::get<N>(to_tuple_view(std::forward<decltype(value)>(value)));
    }
    // get<T>
    template <typename T>
    [[nodiscard]] constexpr decltype(auto) get(concepts::aggregate auto && value) noexcept {
    // using indexes here rather than type, to avoid collisions of cvref-qualified view elements
        using tuple_t = to_tuple_t<std::remove_cvref_t<decltype(value)>>;
        constexpr auto index = details::mp::first_index_of_v<T, tuple_t>;
        return get<index>(std::forward<decltype(value)>(value));
    }

    // --- conversions ---
    // tuple conversion / tie (strict field conversions: same possibly-cvref-qualified types)
    //  ex: struct type{ A v0; B & v1; const C && v2 } => std::tuple<A, B&, const C&&>;
    [[nodiscard]] constexpr auto to_tuple(concepts::aggregate auto && value) {
        using value_type = std::remove_cvref_t<decltype(value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
            using result_t = std::tuple<
                csl::ag::element_t<indexes, value_type>...
            >;
            return result_t{
                csl::ag::get<indexes>(csl_fwd(value))...
            };
        }(std::make_index_sequence<size_v<value_type>>{});
    }

    // conversion factory. unfold into an either complete or template type T
    // interally performs get<indexes>...
    // REFACTO: universal template
    // REFACTO: apply
    template <typename T>
    [[nodiscard]] constexpr auto make(csl::ag::concepts::aggregate auto && from_value) {
        using type = std::remove_cvref_t<decltype(from_value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return T{ csl::ag::get<indexes>(csl_fwd(from_value))... };
        }(std::make_index_sequence<csl::ag::size_v<type>>{});
    }
    template <template <typename...> typename T>
    [[nodiscard]] constexpr auto make(csl::ag::concepts::aggregate auto && from_value) {
        using type = std::remove_cvref_t<decltype(from_value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return T{ csl::ag::get<indexes>(csl_fwd(from_value))... };
        }(std::make_index_sequence<csl::ag::size_v<type>>{});
    }
    template <template <typename, auto ...> typename T>
    [[nodiscard]] constexpr auto make(csl::ag::concepts::aggregate auto && from_value) {
        using type = std::remove_cvref_t<decltype(from_value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return T{ csl::ag::get<indexes>(csl_fwd(from_value))... };
        }(std::make_index_sequence<csl::ag::size_v<type>>{});
    }
    template <template <auto, typename ...> typename T>
    [[nodiscard]] constexpr auto make(csl::ag::concepts::aggregate auto && from_value) {
        using type = std::remove_cvref_t<decltype(from_value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return T{ csl::ag::get<indexes>(csl_fwd(from_value))... };
        }(std::make_index_sequence<csl::ag::size_v<type>>{});
    }

    // TODO(Guss)
    // conversion factory. unfold into an either complete or template type T
    // interally performs get<Ts>... (requires unique<Ts...>)
    // motivation: struct { int; string } => struct { string; int }
    //
    // use type-qualifier/decorator orderer/unordered ?
    // ex:
    //  auto other = value | views::move | views::unordered | to<other_type>; // -> get<Ts...>
    //
    
    // TODO(Guss)
    //  move_view -> or already equivalent to std::move(value) | views::smthg ? (TO TEST)
}
namespace csl::ag::concepts {
    template <typename T, typename U>
    concept convertible_to = requires{ csl::ag::make<U>(std::declval<T>()); };

    // size-related constraints
    template <typename T>
    concept empty = aggregate<T> and csl::ag::empty_v<T>;
    // template <typename T, std::size_t N>
    // concept sized_by = aggregate<T> and csl::ag::size_v<T> == N;
    // template <typename T, std::size_t N>
    // concept greater_than = aggregate<T> and csl::ag::size_v<T> > N;
    // template <typename T, std::size_t N>
    // concept greater_or_eq = aggregate<T> and csl::ag::size_v<T> >= N;
}
// --- DSL ---
namespace csl::ag {
// ADL-used
    // view: all
    struct all_view_tag{};
    [[nodiscard]] constexpr inline static auto operator|(csl::ag::concepts::aggregate auto && value, const csl::ag::all_view_tag &)
    {
        return csl::ag::to_tuple_view(csl_fwd(value));
    }

    // conversion: common
    // POC: https://godbolt.org/z/Yc5no5MzP

    // QUESTION: specific tag to allow narrowing-conversions ? (injects static_cast - as a projection ?)

    // conversion
    // REFACTO: REFACTO: P1950 Universal Template Paramters
    template <typename T>
    struct to_complete_type_tag{};
    template <template <typename...> typename>
    struct to_template_type_ttps_tag{};
    template <template <typename, auto ...> typename>
    struct to_template_type_ttp_nttps_tag{};
    template <template <auto, typename ...> typename>
    struct to_template_type_nttp_ttps_tag{};

    template <typename T>
    constexpr inline static auto to(){ return to_complete_type_tag<T>{}; };
    template <template <typename...> typename T>
    constexpr inline static auto to(){ return to_template_type_ttps_tag<T>{}; };
    template <template <typename, auto...> typename T> 
    constexpr inline static auto to(){ return to_template_type_ttp_nttps_tag<T>{}; };
    template <template <auto, typename ...> typename T>
    constexpr inline static auto to(){ return to_template_type_nttp_ttps_tag<T>{}; };

    template <typename T>
    [[nodiscard]] constexpr inline static auto operator|(csl::ag::concepts::aggregate auto && value, to_complete_type_tag<T>)
    {
        return csl::ag::make<T>(csl_fwd(value));
    }
    template <template <typename...> typename T>
    [[nodiscard]] auto operator|(csl::ag::concepts::aggregate auto && value, to_template_type_ttps_tag<T>)
    {
        return csl::ag::make<T>(csl_fwd(value));
    }
    template <template <typename, auto ...> typename T>
    [[nodiscard]] auto operator|(csl::ag::concepts::aggregate auto && value, to_template_type_ttp_nttps_tag<T>)
    {
        return csl::ag::make<T>(csl_fwd(value));
    }
    template <template <auto, typename ...> typename T>
    [[nodiscard]] auto operator|(csl::ag::concepts::aggregate auto && value, to_template_type_nttp_ttps_tag<T>)
    {
        return csl::ag::make<T>(csl_fwd(value));
    }
}
namespace csl::ag::views {
    constexpr inline static auto all = all_view_tag{};
    template <typename T>
    using all_t = decltype(std::declval<T>());

    // TODO(Guillaume): common_t -> std::tuple<std::common_type<Ts>...>

    // TODO(Guillaume): #245: flatten_view
}
// --- opt-ins ---
// TODO(Guillaume): REFACTO, tests ?
// - ticket: better test coverage
// - ticket: update documentation
// TODO(Guillaume): hash, compare, assign?, etc.
namespace csl::ag::details::options::detection {
    template <typename T, typename = void> struct std_tuple_interface : std::false_type {};
    template <typename T> struct std_tuple_interface<T, typename T::csl_optins::ag::std_tuple_interface> : std::true_type {};
    template <typename T> constexpr inline static auto std_tuple_interface_v = std_tuple_interface<T>::value;
}
namespace csl::ag::concepts {
    template <typename T>
    concept opt_in_std_tuple_interface =
        concepts::aggregate<std::remove_cvref_t<T>>
    and csl::ag::details::options::detection::std_tuple_interface_v<std::remove_cvref_t<T>>
    ;
}
// --- functional API ---
#include <functional>
namespace csl::ag::details {
    template <std::size_t ... indexes>
    constexpr decltype(auto) apply_impl(auto && f, csl::ag::concepts::aggregate auto && value, std::index_sequence<indexes...>)
    noexcept(noexcept(std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value))...)))
    {
        return std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value))...);
    }
    template <std::size_t ... indexes>
    constexpr void for_each_impl(auto && f, csl::ag::concepts::aggregate auto && value, std::index_sequence<indexes...>)
    noexcept(noexcept((std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value))), ...)))
    {
        ((std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value))), ...));
    }
}
namespace csl::ag {

    constexpr decltype(auto) apply(auto && f, csl::ag::concepts::aggregate auto && value)
    noexcept(
        noexcept(
            details::apply_impl(
                csl_fwd(f),
                csl_fwd(value),
                std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<decltype(value)>>>{}
            )
        )
    )
    {
        return details::apply_impl(
            csl_fwd(f),
            csl_fwd(value),
            std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<decltype(value)>>>{}
        );
    }

    template <typename F>
    constexpr void for_each(F && f, csl::ag::concepts::aggregate auto && value)
    noexcept(
        noexcept(
            details::for_each_impl(
                csl_fwd(f),
                csl_fwd(value),
                std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<decltype(value)>>>{}
            )
        )
    )
    {
        return details::for_each_impl(
            csl_fwd(f),
            csl_fwd(value),
            std::make_index_sequence<csl::ag::size_v<std::remove_cvref_t<decltype(value)>>>{}
        );
    }
}

// ---------------------
//  formatting/printing
// ---------------------

// QUESTION(Guss): opt-in -> include as an extra file:
//  csl/ag/features/io.hpp
//  - csl/ag/features/io/fmtlib.hpp
//  - csl/ag/features/io/std_format.hpp
//  - csl/ag/features/io/iostream.hpp -> should internally use std::format or fmt::format, if availble

#if defined(CSL_AG__ENABLE_IOSTREAM_SUPPORT) and CSL_AG__ENABLE_IOSTREAM_SUPPORT

static_assert(false, "(experimentale) CSL_AG__ENABLE_IOSTREAM_SUPPORT feature is disabled for now");

// csl::ag::io
// REFACTO: #134
#include <string_view>

namespace gcl::io {
    using abs = gcl::pattern::strong_type<std::size_t, struct indent_abs_t>;
    using rel = gcl::pattern::strong_type<int,         struct indent_rel_t>;
}

#include <iomanip>
#include <cassert>

namespace gcl::io::details {
    struct line{
        const std::size_t indent_value = 0; // NOLINT
        // factories
        constexpr auto operator()(io::abs value) const noexcept {
            return line{ indent_value + value };
        }
        constexpr auto operator()(io::rel value) const noexcept {
            assert(indent_value + value > 0); // NOLINT
            return line{ indent_value + value };
        }
    };
}

#include <limits>

namespace gcl::io {
    constexpr inline static auto indent = details::line{};

    class indented_ostream {

		// TODO(Guss): as style
		//	+ break-after-brace
		constexpr inline static size_t indent_width = 3;
		std::ostream & bounded_ostream;
        const std::size_t depth = 0;

    public:
		[[nodiscard]] auto & bounded_to() const {
			return bounded_ostream;
		}

        indented_ostream() noexcept = delete;
        indented_ostream(std::ostream & output_stream, std::size_t initial_depth = 0) noexcept // NOLINT(google-explicit-constructor)
        : bounded_ostream{ output_stream }
        , depth{ initial_depth }
        {}
        indented_ostream(const indented_ostream& other) noexcept
        : bounded_ostream { other.bounded_ostream }
        , depth { other.depth + 1 }
        {}
		indented_ostream(indented_ostream&&) noexcept = default;
		~indented_ostream() noexcept = default;

		indented_ostream & operator=(const indented_ostream & other) noexcept = delete;
		indented_ostream & operator=(indented_ostream&&) noexcept = delete;

        auto operator<<(const auto & value) const -> const indented_ostream & {
            bounded_ostream << value;
            return *this;
        }
        auto operator<<(const details::line l) const -> const indented_ostream & {
			assert( // NOLINT
				std::max(depth, l.indent_value) - std::min(depth, l.indent_value)
				<= (std::numeric_limits<int>::max() / indent_width)
			);
            bounded_ostream << std::setw((depth + l.indent_value) * indent_width) << "";
            return *this;
        }
    };
}

namespace csl::ag::io {
	//  For GCC [10.3 .. 12.1] : Can't use the following synthax anymore (constraint depends on itself)
    //  (might be same issue as https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99599)
    //      std::ostream & operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      requires (not ostream_shiftable<decltype(value)>::value)
    //  nor delayed adl :
    //      std::ostream & details::ostream_shift(std::ostream & os, csl::ag::concepts::aggregate auto const& value)
    //      requires (not ostream_shiftable<decltype(value)>::value); // never defined
    //
    //      auto operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      -> decltype(details::ostream_shift(os, value))
    //
    // Warning : csl::ag makes aggregate type tuplelike
	auto & operator<<(const gcl::io::indented_ostream os, csl::ag::concepts::structured_bindable auto && value)
    requires (not std::is_array_v<std::remove_cvref_t<decltype(value)>>) // temporary quickfix
    {
        using value_type = std::remove_cvref_t<decltype(value)>;

        constexpr auto size = []() constexpr { // work-around for ADL issue
            // TODO(Guss): unqualified tuple_size_v lookup instead here
            if constexpr (csl::ag::concepts::tuple_like<value_type>)
                return std::tuple_size_v<value_type>;
            else if constexpr (csl::ag::concepts::aggregate<value_type>)
                return csl::ag::size_v<value_type>;
            else
                static_assert(sizeof(value_type) and false, "csl::ag::print : invalid type"); // NOLINT
        }();

        using namespace gcl::io;
        os << gcl::cx::type_name_v<decltype(value)> << " : {\n";

        const auto print_value = [&]<size_t index>(){
            os << indent(rel(1)) << '[' << index << "] ";
            auto && element_value = std::get<index>(std::forward<decltype(value)>(value));
            using element_value_type = std::tuple_element_t<index, std::remove_cvref_t<decltype(value)>>; //decltype(element_value);
            if constexpr (not csl::ag::concepts::structured_bindable<element_value_type>)
                os << gcl::cx::type_name_v<element_value_type> << " : ";
            os << std::forward<decltype(element_value)>(element_value) << '\n';
        };

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((print_value.template operator()<indexes>()), ...);  
        }(std::make_index_sequence<size>{});

        os << indent << "}";
        return os.bounded_to();
    }
}
#endif // CSL_AG__ENABLE_IOSTREAM_SUPPORT

// Opt-in fmt support
//	wip : https://godbolt.org/z/Enj5nTzj6
//  wip (presentation) : https://godbolt.org/z/qfTMoT7fo
//		see https://github.com/GuillaumeDua/CppShelf/issues/57
#if defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) and not __has_include(<fmt/format.h>)
    static_assert(false, "csl::ag: [CSL_AG_ENABLE_FMTLIB_SUPPORT] set to [true], but header <fmt/format.h> is missing. Did you forget a dependency ?");
#elif defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)

# include <fmt/ranges.h>
# include <fmt/compile.h>

namespace csl::ag::details::mp {
	template <typename>
	struct is_std_array : std::false_type{};
	template <typename T, std::size_t N>
	struct is_std_array<std::array<T, N>> : std::true_type{};
}
namespace csl::ag::io::concepts {
	template <typename T>
	concept formattable = 
		csl::ag::concepts::aggregate<T> and
		(not std::is_array_v<T>) and
		(not csl::ag::details::mp::is_std_array<T>::value)
        // QUESTION: and not is_std_tuple ?
	;
}
# include <variant>

namespace csl::ag::io::presentation {
    template <typename Char>
    struct none {
        fmt::basic_string_view<Char>
            separator,
            opening_bracket, closing_bracket,
            indentation
        ;
    };
    template <typename Char>
    struct compact {
        fmt::basic_string_view<Char>
            separator = ", ",
            opening_bracket = "{",
            closing_bracket = "}",
            indentation
        ;
    };
    template <typename Char>
    struct indented {

        [[nodiscard]] constexpr inline static auto make_indentation(std::size_t depth){
            return std::string(depth * 3, ' ');
        }

        explicit constexpr indented(std::size_t depth)
        : indentation(make_indentation(depth + 1))
        , closing_bracket("\n" + make_indentation(depth) + "}")
        {}

        // NOLINTBEGIN(*-non-private-member-variables-in-classes)
        fmt::basic_string_view<Char> separator = ",\n";
        fmt::basic_string_view<Char> opening_bracket = "{\n";
        std::basic_string<Char> indentation;
        std::basic_string<Char> closing_bracket;
        // NOLINTEND(*-non-private-member-variables-in-classes)
    };

    template <typename Char>
    using type = std::variant<compact<Char>, indented<Char>, none<Char>>;
}

// TODO(Guillaume): extra opt-in tag-dispatch to force such an instanciation, so it does not clash with tuplelikes, etc.
// QUESTION: use string-formatter, with filler + width ?
// QUESTION: unqualified get (possibly as an opt-in/parse-parameter ? -> supports std::tuple, std::array, etc. ?
template <csl::ag::io::concepts::formattable T, class Char>
struct fmt::formatter<T, Char>
{
    using csl_product = void;
private:

    static auto deduce_formatters_type() -> csl::ag::concepts::tuple_like auto {
        return []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::tuple<
                fmt::formatter<
                    csl::ag::element_t<indexes, T>
                >...
            >{};
        }(std::make_index_sequence<csl::ag::size_v<T>>{});
    }
    std::remove_cvref_t<decltype(deduce_formatters_type())> formatters{};

    csl::ag::io::presentation::type<Char> style = csl::ag::io::presentation::compact<Char>{};
    std::size_t depth{ 0 }; // product -> other impl, or template-specialization

    #if false // csl::typeinfo enabled
    // decorations
    bool typenamed{ false };
    template <typename T, typename FormatContext>
    void format_typename(FormatContext & ctx) const {
        if (not typenamed)
            return;

        ctx.advance_to(detail::copy<Char>(
            fmt::string_view{csl::typeinfo::type_name_v<T>},
            ctx.out())
        );
        ctx.advance_to(detail::copy<Char>(fmt::string_view{": "}, ctx.out()));
    }
    #endif

    bool indexed{ false };
    template <std::size_t index, typename FormatContext>
    void format_index(FormatContext & ctx) const {
        if (not indexed)
            return;
        fmt::format_to(ctx.out(), "[{}] ", index);
    }

    template <std::size_t index, typename FormatContext>
    auto format_element(const auto & value, FormatContext & ctx, const auto & style_alternative) const -> decltype(ctx.out()){

        if (index > 0) ctx.advance_to(detail::copy<Char>(style_alternative.separator, ctx.out()));
        ctx.advance_to(detail::copy<Char>(fmt::string_view{style_alternative.indentation}, ctx.out()));

        format_index<index>(ctx);
        auto && element = std::get<index>(value);
        format_typename<decltype(element)>(ctx);
        return std::get<index>(formatters).format(element, ctx);
    }

public:
    // requires P2893 - variadic friends: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2893r2.html
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx, std::size_t depth) -> decltype(ctx.begin()) {
        this->depth = depth;
        return parse(ctx);
    }

// public:

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        // Question: spread only the rest ?
        //  or use a 's' spread policy ?
        //  or a maximum depth ?
        const auto parse_element = [&]<std::size_t index>(){

            using element_t = std::tuple_element_t<index, T>;
            if constexpr (not csl::ag::concepts::aggregate<std::remove_cvref_t<element_t>>)
                return;
            else {
                auto end1 = std::get<index>(formatters).parse(ctx, depth + 1);
                // if (end1 not_eq it)
                //     report_error("invalid format specs for tuple elements");
            }
        };
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((parse_element.template operator()<indexes>()), ...);
        }(std::make_index_sequence<csl::ag::size_v<T>>{});

        while (it not_eq end and *it not_eq '}')
        {
            switch (detail::to_ascii(*it)){
                // style
                case 'n': style = csl::ag::io::presentation::none<Char>{}; break;
                case 'c': style = csl::ag::io::presentation::compact<Char>{}; break;
                case 'i': style = csl::ag::io::presentation::indented<Char>{depth}; break;
                case ',': break;
                // projection
                case 'I': indexed = true; break;
                // case 'T': typenamed = true; break; // Q: max-depth ? cvref-qualifiers ?
                default: report_error("invalid format specifier");
            }
            ++it;
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const T & value, FormatContext& ctx) const -> decltype(ctx.out()) {

        if (depth == 0)
            format_typename<decltype(value)>(ctx);

        return std::visit([&](const auto & style_alternative) -> decltype(ctx.out()){

            auto out = ctx.out();
            ctx.advance_to(detail::copy<Char>(style_alternative.opening_bracket, ctx.out()));

            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                ((format_element<indexes>(value, ctx, style_alternative)), ...);
            }(std::make_index_sequence<csl::ag::size_v<T>>{});

            ctx.advance_to(detail::copy<Char>(fmt::string_view{style_alternative.closing_bracket}, ctx.out()));
            return ctx.out();
        }, style);
    }
};

namespace csl::ag::concepts {
    template <typename T>
    concept csl_product = requires {
        typename T::csl_product;
    };
}

#endif // CSL_AG__ENABLE_FMTLIB_SUPPORT

// TODO(Guss) Opt-in std::format support
#if defined(CSL_AG__ENABLE_FORMAT_SUPPORT) and not __has_include(<format>)
    static_assert(false, "csl::ag: [CSL_AG_ENABLE_STD_FORMAT_SUPPORT] set to [true], but header <format> is missing. Did you forget a dependency ?");
#elif defined(CSL_AG__ENABLE_FORMAT_SUPPORT)
#endif // CSL_AG__ENABLE_FORMAT_SUPPORT

// TODO(Guss): for_each(_fields)(aggregate auto &&, visitor F&&)
//  [ ] std::hash
//  [ ] comparator
//  [ ] projections
// TODO(Guss): opt-in(s) ?
//  [ ] operator==
//  [ ] operator= / assign

#undef csl_fwd
