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

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ag::details {

    template <typename T>
    constexpr auto declval() noexcept -> std::add_rvalue_reference_t<T>  {
        // static_assert([](){ return false; }(), "csl::ag::details : declval : for unevaluated context only !");
        if constexpr (std::is_lvalue_reference_v<T>)
            return *((std::remove_reference_t<T>*){ nullptr });
        else
            return std::move(*((std::remove_reference_t<T>*){ nullptr }));
    }

    template <std::size_t>
    struct field_evaluator {
        explicit constexpr field_evaluator() = delete;

		// Implicit conversion
        // 	not `return std::declval<T>();`, as clang does not like it
        // 	neither `consteval` -> Clang ICE
        template <typename T>
        [[nodiscard]] constexpr operator T&&() const noexcept { // NOLINT(google-explicit-constructor)
            return declval<T&&>();
        }
        template <typename T>
        [[nodiscard]] constexpr operator T&() const noexcept { // NOLINT(google-explicit-constructor)
            return declval<T&>();
        }
    };
}
namespace csl::ag::details::mp {
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

    // add cv - P1450 impl detail (also for ref-qualified types)
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
}
namespace csl::ag::details {
    template <typename owner, typename T>
    // T should be explicit
    constexpr auto make_field_view(T && value) -> mp::field_view_t<owner, T> {
        static_assert(std::is_reference_v<mp::field_view_t<owner, T>>);
        if constexpr (std::is_lvalue_reference_v<mp::field_view_t<owner, T>>)
            return *(&value);
        else return csl_fwd(value);
    }
    template <typename owner, typename ... Ts>
    constexpr auto make_tuple_view(Ts&& ... values) {
        using view_type = std::tuple<mp::field_view_t<owner, Ts>...>;
        return view_type{ make_field_view<owner, Ts>(csl_fwd(values))... };
    }
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
            return concepts::aggregate_constructible_from<T, details::field_evaluator<indexes>...>;
        }(std::make_index_sequence<size>{})
    ;

	template <typename T>
    concept tuplelike =
        requires { std::tuple_size<std::remove_reference_t<T>>{}; }
    ;
	template <typename T>
	concept structured_bindable = tuplelike<T> or aggregate<T>;
}
namespace csl::ag::details {

    #if not defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
    # pragma message("csl::ag : CSL_AG_ENABLE_BITFIELDS_SUPPORT [disabled], faster algorithm selected")
	template <concepts::aggregate T, std::size_t indice>
    requires (std::default_initializable<T>)
    consteval auto fields_count_impl() -> std::size_t {
    // faster algorithm if T is default_initializable (ref fields can be initialized),
    // and no fields is a bitfield.
        static_assert(not std::is_reference_v<T>);

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
    # pragma message("csl::ag : CSL_AG_ENABLE_BITFIELDS_SUPPORT [enabled], slower algorithm selected")
    #endif

    template <concepts::aggregate T, std::size_t indice>
    consteval auto fields_count_impl() -> std::size_t {
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
    constexpr std::size_t fields_count = fields_count_impl<
        T,
        sizeof(T)
        #if defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
        * sizeof(std::byte) * CHAR_BIT
        #endif
    >();

	template <std::size_t N, concepts::aggregate T>
	struct element;

// GENERATED CONTENT, DO NOT EDIT MANUALLY !
#pragma region as_tuple_view_impl<N,T>
template <std::size_t N> requires (N == 1) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0 ] = value;
	return make_tuple_view<decltype(value), decltype(v0)>( csl_fwd(v0) );
}
template <std::size_t N> requires (N == 2) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1)>( csl_fwd(v0),csl_fwd(v1) );
}
template <std::size_t N> requires (N == 3) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2) );
}
template <std::size_t N> requires (N == 4) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3) );
}
template <std::size_t N> requires (N == 5) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4) );
}
template <std::size_t N> requires (N == 6) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5) );
}
template <std::size_t N> requires (N == 7) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6) );
}
template <std::size_t N> requires (N == 8) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7) );
}
template <std::size_t N> requires (N == 9) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8) );
}
template <std::size_t N> requires (N == 10) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9) );
}
template <std::size_t N> requires (N == 11) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10) );
}
template <std::size_t N> requires (N == 12) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11) );
}
template <std::size_t N> requires (N == 13) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12) );
}
template <std::size_t N> requires (N == 14) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13) );
}
template <std::size_t N> requires (N == 15) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14) );
}
template <std::size_t N> requires (N == 16) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15) );
}
template <std::size_t N> requires (N == 17) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16) );
}
template <std::size_t N> requires (N == 18) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17) );
}
template <std::size_t N> requires (N == 19) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18) );
}
template <std::size_t N> requires (N == 20) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19) );
}
template <std::size_t N> requires (N == 21) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20) );
}
template <std::size_t N> requires (N == 22) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21) );
}
template <std::size_t N> requires (N == 23) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22) );
}
template <std::size_t N> requires (N == 24) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23) );
}
template <std::size_t N> requires (N == 25) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24) );
}
template <std::size_t N> requires (N == 26) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25) );
}
template <std::size_t N> requires (N == 27) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26) );
}
template <std::size_t N> requires (N == 28) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27) );
}
template <std::size_t N> requires (N == 29) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28) );
}
template <std::size_t N> requires (N == 30) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29) );
}
template <std::size_t N> requires (N == 31) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30) );
}
template <std::size_t N> requires (N == 32) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = value;
	return make_tuple_view<decltype(value), decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31)>( csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30),csl_fwd(v31) );
}
#pragma endregion
#pragma region element<N, T>
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 1) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0 ] = declval<T>();
            return std::tuple<decltype(v0)>{ csl_fwd(v0) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 2) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1)>{ csl_fwd(v0),csl_fwd(v1) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 3) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 4) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 5) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 6) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 7) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 8) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 9) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 10) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 11) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 12) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 13) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 14) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 15) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 16) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 17) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 18) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 19) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 20) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 21) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 22) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 23) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 24) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 25) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 26) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 27) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 28) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 29) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 30) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 31) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 32) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31)>{ csl_fwd(v0),csl_fwd(v1),csl_fwd(v2),csl_fwd(v3),csl_fwd(v4),csl_fwd(v5),csl_fwd(v6),csl_fwd(v7),csl_fwd(v8),csl_fwd(v9),csl_fwd(v10),csl_fwd(v11),csl_fwd(v12),csl_fwd(v13),csl_fwd(v14),csl_fwd(v15),csl_fwd(v16),csl_fwd(v17),csl_fwd(v18),csl_fwd(v19),csl_fwd(v20),csl_fwd(v21),csl_fwd(v22),csl_fwd(v23),csl_fwd(v24),csl_fwd(v25),csl_fwd(v26),csl_fwd(v27),csl_fwd(v28),csl_fwd(v29),csl_fwd(v30),csl_fwd(v31) };
        }())>>
    {};
#pragma endregion
// END OF GENERATED CONTENT

    template <std::size_t>
    constexpr auto as_tuple_view_impl(concepts::aggregate auto &&) {
        static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
    }
}

namespace csl::ag {

    // tuple-view
    // factory that creates a lightweight accessor to an existing aggregate value,
    // preserving cvref semantic
    constexpr auto as_tuple_view(concepts::aggregate auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::as_tuple_view_impl<details::fields_count<type>>(std::forward<decltype(value)>(value));
    }
    template <concepts::aggregate T> requires (std::is_reference_v<T>)
    struct tuple_view : std::type_identity<decltype(as_tuple_view(std::declval<T>()))>{}; 
    template <concepts::aggregate T> requires (std::is_reference_v<T>)
    using tuple_view_t = typename tuple_view<T>::type;

    // view_element
	template <std::size_t N, concepts::aggregate T>
    requires (std::is_reference_v<T>)
    struct view_element : std::tuple_element<N, tuple_view_t<T>>{};
    // struct view_element : std::tuple_element<N, decltype(as_tuple_view(std::declval<T>()))>{};
	template <std::size_t N, concepts::aggregate T>
	using view_element_t = typename view_element<N, T>::type;

	// get
    template <std::size_t N>
    constexpr decltype(auto) get(concepts::aggregate auto && value) {
        return ::std::get<N>(as_tuple_view(std::forward<decltype(value)>(value)));
    }

	// element
	template <std::size_t N, concepts::aggregate T>
    using element = details::element<N, T>;
	template <std::size_t N, concepts::aggregate T>
	using element_t = typename element<N, T>::type;

    // size
    template <csl::ag::concepts::aggregate T>
    struct size : std::integral_constant<std::size_t, details::fields_count<std::remove_reference_t<T>>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr inline auto size_v = size<T>::value;

    // tuple conversion (not view !)
    constexpr auto as_tuple(concepts::aggregate auto && value) {
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
    template <concepts::aggregate T> requires (not std::is_reference_v<T>)
    using to_tuple = std::type_identity<decltype(as_tuple(std::declval<T>()))>;
    template <concepts::aggregate T> requires (not std::is_reference_v<T>)
    using to_tuple_t = typename to_tuple<T>::type;
}
// tuple-like interface
//  N4606 [namespace.std]/1 :
//  A program may add a template specialization for any standard library template to namespace std
//  only if the declaration depends on a user-defined type 
//  and the specialization meets the standard library requirements for the original template and is not explicitly prohibited.
namespace std { // NOLINT(cert-dcl58-cpp)
// TODO(Guss) : as opt-in, so aggregate are not necessarily tuplelike

    template <std::size_t N>
    constexpr decltype(auto) get(::csl::ag::concepts::aggregate auto && value) {
        return std::get<N>(csl::ag::as_tuple_view(std::forward<decltype(value)>(value)));
    }

    template <std::size_t N, ::csl::ag::concepts::aggregate T>
    struct tuple_element<N, T> : ::csl::ag::element<N, T>{};

    // // screw-up the ADL (aggregate structured-binding vs tuplelike)
    // template <csl::ag::concepts::aggregate T>
    // struct tuple_size<T> : std::integral_constant<std::size_t, csl::ag::details::fields_count<T>>{};
}

// csl::ag::io
#include <string_view>

// TODO: remove this coupling with gcl
namespace gcl::cx::details {
    struct type_prefix_tag { constexpr static std::string_view value = "T = "; };
    struct value_prefix_tag { constexpr static std::string_view value = "value = "; };

    template <typename prefix_tag_t>
    static constexpr auto parse_mangling(std::string_view value, std::string_view function) {
        value.remove_prefix(value.find(function) + function.size());
    #if defined(__GNUC__) or defined(__clang__)
            value.remove_prefix(value.find(prefix_tag_t::value) + std::size(prefix_tag_t::value));
        #if defined(__clang__)
            value.remove_suffix(value.length() - value.rfind(']'));
        #elif defined(__GNUC__) // GCC
            value.remove_suffix(value.length() - value.find(';'));
        #endif
    #elif defined(_MSC_VER)
        if (auto enum_token_pos = value.find("enum "); enum_token_pos == 0)
            value.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        value.remove_suffix(value.length() - value.rfind(">(void)"));
    #endif
        return value;
    }
}
namespace gcl::cx {
    template <typename T>
    static consteval auto type_name()
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::type_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
    #elif defined(_MSC_VER)
        return details::parse_mangling<details::type_prefix_tag>(__FUNCSIG__, __func__);
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
    }
    template <typename T>
    constexpr inline auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr auto type_name()
    -> std::string_view
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr auto value_name()
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::value_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
    #elif defined(_MSC_VER)
        return details::parse_mangling<details::value_prefix_tag>(__FUNCSIG__, __func__);
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
    }
    template <auto value>
    constexpr inline auto value_name_v = value_name<value>();
}
// TODO: remove this coupling with gcl
namespace gcl::pattern
{
	template <typename T, typename>
    struct strong_type
    {
        using underlying_type = T;
        using reference = T&;
        using const_reference = const T &;

        constexpr explicit strong_type(const_reference arg)
        requires std::copy_constructible<T>
        : value(arg)
        {}
        constexpr explicit strong_type(T&& arg)
        requires std::move_constructible<T>
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        [[nodiscard]] constexpr reference       underlying()        { return value; }
        [[nodiscard]] constexpr const_reference underlying() const  { return value; }

		// Implicit conversions
        [[nodiscard]] constexpr operator reference ()               { return underlying(); } /* NOLINT(google-explicit-constructor)*/
        [[nodiscard]] constexpr operator const_reference () const   { return underlying(); } /* NOLINT(google-explicit-constructor)*/

    private:
        T value;
    };
}
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
    constexpr inline auto indent = details::line{};

    class indented_ostream {

		// TODO(Guss): as style
		//	+ break-after-brace
		constexpr static size_t indent_width = 3;
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
            if constexpr (csl::ag::concepts::tuplelike<value_type>)
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

// fmt
//	wip : https://godbolt.org/z/7b1Ga168P
//  wip (presentation) : https://godbolt.org/z/qfTMoT7fo
//		see https://github.com/GuillaumeDua/CppShelf/issues/57
#ifdef FMT_FORMAT_H_
# include <fmt/ranges.h>

namespace csl::ag::details::mp {
	template <typename>
	struct is_std_array : std::false_type{};
	template <typename T, std::size_t N>
	struct is_std_array<std::array<T, N>> : std::true_type{};
}
namespace csl::ag::details::concepts {
	template <typename T>
	concept formattable_aggregate = 
		csl::ag::concepts::aggregate<T> and
		(not std::is_array_v<T>) and
		(not is_std_array<T>::value)
	;
}

// TODO(Guss) : opt-in (include as an extra file : csl::ag::io)
template <formattable_aggregate T, class CharT>
struct fmt::formatter<T, CharT>
{
    // TODO(Guss)
    char presentation = 'c'; // [c:compact, pN:pretty (where N is the depth level)]

    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {

        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'c' || *it == 'p'))
            presentation = *it++;
        if (it != end && *it != '}')
            throw fmt::format_error{"invalid format"};

        return it;
    }

    // or : return format_to(out, "{}", csl::ag::as_tuple_view(value));
    template <typename FormatContext>
    constexpr auto format(const T & value, FormatContext& ctx)
    {
        auto&& out = ctx.out();
        constexpr auto size = csl::ag::size_v<std::remove_cvref_t<T>>;
        if (size == 0)
            return out;
        *out++ = '{';
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            (
                format_to(
                    out, "{}{}",
                    csl::ag::get<indexes>(value),
                    (indexes == (size - 1) ? "" : ", ")
                )
            , ...);
        }(std::make_index_sequence<size>{});
        *out++ = '}';
        return out;
    }
};

#undef csl_fwd

#endif
