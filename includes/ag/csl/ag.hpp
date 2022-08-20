#pragma once

#include <array>
#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>
#include <climits>
#include <string_view>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

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
    // apply_ref
    template <typename from, typename to>
    struct apply_ref : std::remove_reference<to>{};
    template <typename from, typename to>
    struct apply_ref<from&, to> : std::add_lvalue_reference<to>{};
    template <typename from, typename to>
    struct apply_ref<from&&, to> : std::add_rvalue_reference<std::remove_reference_t<to>>{};
    template <typename from, typename to>
    using apply_ref_t = typename apply_ref<from, to>::type;

    // add cv (also for ref-qualified types)
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

    // apply_cv
    template <typename from, typename to>
    struct apply_cv : std::remove_cv<to>{};
    template <typename from, typename to> requires (std::is_reference_v<from>)
    struct apply_cv<from, to> : apply_cv<std::remove_reference_t<from>, to>{};
    template <typename from, typename to>
    struct apply_cv<const volatile from, to> : add_cv<to>{};
    template <typename from, typename to>
    struct apply_cv<const from, to> : add_const<to>{};
    template <typename from, typename to>
    struct apply_cv<volatile from, to> : add_volatile<to>{};
    template <typename from, typename to>
    using apply_cv_t = typename apply_cv<from, to>::type;

    // apply_cvref
    template <typename from, typename to>
    struct apply_cvref : apply_cv<from, apply_ref_t<from, to>>{};
    template <typename from, typename to>
    using apply_cvref_t = typename apply_cvref<from, to>::type;

    // field_view
    template <typename owner, typename T>
    struct field_view : apply_cvref<owner, T>{};
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
        else return fwd(value);
    }
    template <typename owner, typename ... Ts>
    constexpr auto make_tuple_view(Ts&& ... values) {
        using view_type = std::tuple<mp::field_view_t<owner, Ts>...>;
        return view_type{ make_field_view<owner, Ts>(fwd(values))... };
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
    concept aggregate_with_ref_fields = aggregate<T> and not std::default_initializable<T>;
    template <typename T>
    concept aggregate_without_ref_fields = aggregate<T> and std::default_initializable<T>;

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
	template <concepts::aggregate_without_ref_fields T, std::size_t indice>
    consteval auto fields_count_impl() -> std::size_t {
    // faster algorithm if T does not contains any ref fields,
    // and no fields is a bitfield.
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

    //template <concepts::aggregate_with_ref_fields T, std::size_t indice>
    template <concepts::aggregate T, std::size_t indice>
    consteval auto fields_count_impl() -> std::size_t {
    // costly algorithm
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
	return make_tuple_view<decltype(value)>( fwd(v0) );
}
template <std::size_t N> requires (N == 2) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1) );
}
template <std::size_t N> requires (N == 3) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2) );
}
template <std::size_t N> requires (N == 4) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3) );
}
template <std::size_t N> requires (N == 5) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4) );
}
template <std::size_t N> requires (N == 6) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5) );
}
template <std::size_t N> requires (N == 7) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6) );
}
template <std::size_t N> requires (N == 8) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7) );
}
template <std::size_t N> requires (N == 9) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8) );
}
template <std::size_t N> requires (N == 10) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9) );
}
template <std::size_t N> requires (N == 11) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10) );
}
template <std::size_t N> requires (N == 12) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11) );
}
template <std::size_t N> requires (N == 13) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12) );
}
template <std::size_t N> requires (N == 14) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13) );
}
template <std::size_t N> requires (N == 15) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14) );
}
template <std::size_t N> requires (N == 16) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15) );
}
template <std::size_t N> requires (N == 17) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16) );
}
template <std::size_t N> requires (N == 18) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17) );
}
template <std::size_t N> requires (N == 19) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18) );
}
template <std::size_t N> requires (N == 20) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19) );
}
template <std::size_t N> requires (N == 21) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20) );
}
template <std::size_t N> requires (N == 22) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21) );
}
template <std::size_t N> requires (N == 23) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22) );
}
template <std::size_t N> requires (N == 24) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23) );
}
template <std::size_t N> requires (N == 25) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24) );
}
template <std::size_t N> requires (N == 26) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25) );
}
template <std::size_t N> requires (N == 27) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26) );
}
template <std::size_t N> requires (N == 28) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27) );
}
template <std::size_t N> requires (N == 29) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28) );
}
template <std::size_t N> requires (N == 30) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29) );
}
template <std::size_t N> requires (N == 31) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30) );
}
template <std::size_t N> requires (N == 32) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31) );
}
template <std::size_t N> requires (N == 33) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32) );
}
template <std::size_t N> requires (N == 34) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33) );
}
template <std::size_t N> requires (N == 35) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34) );
}
template <std::size_t N> requires (N == 36) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35) );
}
template <std::size_t N> requires (N == 37) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36) );
}
template <std::size_t N> requires (N == 38) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37) );
}
template <std::size_t N> requires (N == 39) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38) );
}
template <std::size_t N> requires (N == 40) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39) );
}
template <std::size_t N> requires (N == 41) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40) );
}
template <std::size_t N> requires (N == 42) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41) );
}
template <std::size_t N> requires (N == 43) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42) );
}
template <std::size_t N> requires (N == 44) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43) );
}
template <std::size_t N> requires (N == 45) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44) );
}
template <std::size_t N> requires (N == 46) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45) );
}
template <std::size_t N> requires (N == 47) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46) );
}
template <std::size_t N> requires (N == 48) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47) );
}
template <std::size_t N> requires (N == 49) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48) );
}
template <std::size_t N> requires (N == 50) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49) );
}
template <std::size_t N> requires (N == 51) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50) );
}
template <std::size_t N> requires (N == 52) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51) );
}
template <std::size_t N> requires (N == 53) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52) );
}
template <std::size_t N> requires (N == 54) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53) );
}
template <std::size_t N> requires (N == 55) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54) );
}
template <std::size_t N> requires (N == 56) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55) );
}
template <std::size_t N> requires (N == 57) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56) );
}
template <std::size_t N> requires (N == 58) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57) );
}
template <std::size_t N> requires (N == 59) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58) );
}
template <std::size_t N> requires (N == 60) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59) );
}
template <std::size_t N> requires (N == 61) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60) );
}
template <std::size_t N> requires (N == 62) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61) );
}
template <std::size_t N> requires (N == 63) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62) );
}
template <std::size_t N> requires (N == 64) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63) );
}
template <std::size_t N> requires (N == 65) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64) );
}
template <std::size_t N> requires (N == 66) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65) );
}
template <std::size_t N> requires (N == 67) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66) );
}
template <std::size_t N> requires (N == 68) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67) );
}
template <std::size_t N> requires (N == 69) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68) );
}
template <std::size_t N> requires (N == 70) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69) );
}
template <std::size_t N> requires (N == 71) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70) );
}
template <std::size_t N> requires (N == 72) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71) );
}
template <std::size_t N> requires (N == 73) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72) );
}
template <std::size_t N> requires (N == 74) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73) );
}
template <std::size_t N> requires (N == 75) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74) );
}
template <std::size_t N> requires (N == 76) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75) );
}
template <std::size_t N> requires (N == 77) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76) );
}
template <std::size_t N> requires (N == 78) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77) );
}
template <std::size_t N> requires (N == 79) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78) );
}
template <std::size_t N> requires (N == 80) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79) );
}
template <std::size_t N> requires (N == 81) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80) );
}
template <std::size_t N> requires (N == 82) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81) );
}
template <std::size_t N> requires (N == 83) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82) );
}
template <std::size_t N> requires (N == 84) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83) );
}
template <std::size_t N> requires (N == 85) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84) );
}
template <std::size_t N> requires (N == 86) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85) );
}
template <std::size_t N> requires (N == 87) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86) );
}
template <std::size_t N> requires (N == 88) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87) );
}
template <std::size_t N> requires (N == 89) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88) );
}
template <std::size_t N> requires (N == 90) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89) );
}
template <std::size_t N> requires (N == 91) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90) );
}
template <std::size_t N> requires (N == 92) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91) );
}
template <std::size_t N> requires (N == 93) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92) );
}
template <std::size_t N> requires (N == 94) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93) );
}
template <std::size_t N> requires (N == 95) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94) );
}
template <std::size_t N> requires (N == 96) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95) );
}
template <std::size_t N> requires (N == 97) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96) );
}
template <std::size_t N> requires (N == 98) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97) );
}
template <std::size_t N> requires (N == 99) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98) );
}
template <std::size_t N> requires (N == 100) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99) );
}
template <std::size_t N> requires (N == 101) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100) );
}
template <std::size_t N> requires (N == 102) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101) );
}
template <std::size_t N> requires (N == 103) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102) );
}
template <std::size_t N> requires (N == 104) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103) );
}
template <std::size_t N> requires (N == 105) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104) );
}
template <std::size_t N> requires (N == 106) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105) );
}
template <std::size_t N> requires (N == 107) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106) );
}
template <std::size_t N> requires (N == 108) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107) );
}
template <std::size_t N> requires (N == 109) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108) );
}
template <std::size_t N> requires (N == 110) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109) );
}
template <std::size_t N> requires (N == 111) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110) );
}
template <std::size_t N> requires (N == 112) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111) );
}
template <std::size_t N> requires (N == 113) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112) );
}
template <std::size_t N> requires (N == 114) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113) );
}
template <std::size_t N> requires (N == 115) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114) );
}
template <std::size_t N> requires (N == 116) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115) );
}
template <std::size_t N> requires (N == 117) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116) );
}
template <std::size_t N> requires (N == 118) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117) );
}
template <std::size_t N> requires (N == 119) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118) );
}
template <std::size_t N> requires (N == 120) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119) );
}
template <std::size_t N> requires (N == 121) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120) );
}
template <std::size_t N> requires (N == 122) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121) );
}
template <std::size_t N> requires (N == 123) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122) );
}
template <std::size_t N> requires (N == 124) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123) );
}
template <std::size_t N> requires (N == 125) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124) );
}
template <std::size_t N> requires (N == 126) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125) );
}
template <std::size_t N> requires (N == 127) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125),fwd(v126) );
}
template <std::size_t N> requires (N == 128) // NOLINT
 [[nodiscard]] constexpr auto as_tuple_view_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126,v127 ] = value;
	return make_tuple_view<decltype(value)>( fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125),fwd(v126),fwd(v127) );
}
#pragma endregion
#pragma region element<N, T>
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 1) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0 ] = declval<T>();
            return std::tuple<decltype(v0)>{ fwd(v0) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 2) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1)>{ fwd(v0),fwd(v1) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 3) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2)>{ fwd(v0),fwd(v1),fwd(v2) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 4) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 5) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 6) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 7) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 8) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 9) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 10) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 11) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 12) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 13) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 14) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 15) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 16) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 17) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 18) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 19) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 20) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 21) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 22) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 23) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 24) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 25) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 26) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 27) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 28) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 29) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 30) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 31) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 32) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 33) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 34) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 35) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 36) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 37) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 38) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 39) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 40) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 41) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 42) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 43) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 44) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 45) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 46) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 47) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 48) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 49) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 50) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 51) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 52) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 53) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 54) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 55) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 56) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 57) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 58) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 59) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 60) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 61) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 62) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 63) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 64) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 65) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 66) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 67) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 68) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 69) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 70) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 71) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 72) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 73) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 74) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 75) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 76) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 77) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 78) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 79) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 80) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 81) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 82) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 83) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 84) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 85) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 86) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 87) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 88) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 89) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 90) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 91) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 92) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 93) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 94) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 95) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 96) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 97) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 98) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 99) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 100) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 101) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 102) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 103) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 104) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 105) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 106) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 107) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 108) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 109) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 110) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 111) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 112) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 113) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 114) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 115) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 116) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 117) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 118) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 119) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 120) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 121) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 122) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 123) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 124) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 125) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 126) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 127) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125),decltype(v126)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125),fwd(v126) };
        }())>>
    {};
	template <std::size_t N, concepts::aggregate T> requires (fields_count<T> == 128) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126,v127 ] = declval<T>();
            return std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125),decltype(v126),decltype(v127)>{ fwd(v0),fwd(v1),fwd(v2),fwd(v3),fwd(v4),fwd(v5),fwd(v6),fwd(v7),fwd(v8),fwd(v9),fwd(v10),fwd(v11),fwd(v12),fwd(v13),fwd(v14),fwd(v15),fwd(v16),fwd(v17),fwd(v18),fwd(v19),fwd(v20),fwd(v21),fwd(v22),fwd(v23),fwd(v24),fwd(v25),fwd(v26),fwd(v27),fwd(v28),fwd(v29),fwd(v30),fwd(v31),fwd(v32),fwd(v33),fwd(v34),fwd(v35),fwd(v36),fwd(v37),fwd(v38),fwd(v39),fwd(v40),fwd(v41),fwd(v42),fwd(v43),fwd(v44),fwd(v45),fwd(v46),fwd(v47),fwd(v48),fwd(v49),fwd(v50),fwd(v51),fwd(v52),fwd(v53),fwd(v54),fwd(v55),fwd(v56),fwd(v57),fwd(v58),fwd(v59),fwd(v60),fwd(v61),fwd(v62),fwd(v63),fwd(v64),fwd(v65),fwd(v66),fwd(v67),fwd(v68),fwd(v69),fwd(v70),fwd(v71),fwd(v72),fwd(v73),fwd(v74),fwd(v75),fwd(v76),fwd(v77),fwd(v78),fwd(v79),fwd(v80),fwd(v81),fwd(v82),fwd(v83),fwd(v84),fwd(v85),fwd(v86),fwd(v87),fwd(v88),fwd(v89),fwd(v90),fwd(v91),fwd(v92),fwd(v93),fwd(v94),fwd(v95),fwd(v96),fwd(v97),fwd(v98),fwd(v99),fwd(v100),fwd(v101),fwd(v102),fwd(v103),fwd(v104),fwd(v105),fwd(v106),fwd(v107),fwd(v108),fwd(v109),fwd(v110),fwd(v111),fwd(v112),fwd(v113),fwd(v114),fwd(v115),fwd(v116),fwd(v117),fwd(v118),fwd(v119),fwd(v120),fwd(v121),fwd(v122),fwd(v123),fwd(v124),fwd(v125),fwd(v126),fwd(v127) };
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

    constexpr auto as_tuple_view(auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::as_tuple_view_impl<details::fields_count<type>>(std::forward<decltype(value)>(value));
    }

    constexpr auto to_tuple(concepts::aggregate auto && value) {
        auto view = as_tuple_view(std::forward<decltype(value)>(value));
        return [&view]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return std::tuple<std::remove_cvref_t<decltype(std::get<indexes>(view))>...>{
                std::get<indexes>(view)...
            };
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(view)>>>{});
    }

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

	// view_element
	template <std::size_t N, concepts::aggregate T>
    struct view_element : std::tuple_element<N, decltype(as_tuple_view(std::declval<T>()))>{};
	template <std::size_t N, concepts::aggregate T>
	using view_element_t = typename view_element<N, T>::type;

    // size
    template <csl::ag::concepts::aggregate T>
    struct size : std::integral_constant<std::size_t, details::fields_count<T>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr auto size_v = size<T>::value;
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
    static constexpr auto indent = details::line{};

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

#undef fwd

#endif
