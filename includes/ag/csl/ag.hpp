#pragma once

#include <any>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace csl::ag::details {

    template <typename T>
    constexpr auto declval() noexcept -> std::add_rvalue_reference_t<T>  {
        // static_assert([](){ return false; }(), "csl::ag::details : declval : for unevaluated context only !");
        if constexpr (std::is_lvalue_reference_v<std::add_rvalue_reference_t<T>>)
            return *((std::remove_reference_t<T>*){ nullptr });
        else
            return std::move(*((std::remove_reference_t<T>*){ nullptr }));
    }

    template <std::size_t>
    struct field_evaluator {
        explicit constexpr field_evaluator() = delete;

        // not `return std::declval<T>();`, as clang does not like it
        // neither `consteval` -> Clang ICE
        template <typename T>
        constexpr operator T&&() const noexcept {
            return declval<T&&>();
        }
        template <typename T>
        constexpr operator T&() const noexcept {
            return declval<T&>();
        }
    };
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
    template <typename T, typename... args_ts>
    concept aggregate_constructible_from = unqualified_aggregate<T> and requires { T{ std::declval<args_ts>()... }; }
    ;

    template <typename T, std::size_t size>
    concept aggregate_with_n_fields =
        concepts::aggregate<T> and
        []<std::size_t... indexes>(std::index_sequence<indexes...>) {
            return concepts::aggregate_constructible_from<T, details::field_evaluator<indexes>...>;
        }(std::make_index_sequence<size>{})
    ;
}
namespace csl::ag::details {

	template <concepts::aggregate T, std::size_t indice = sizeof(T)>
    requires (indice > 0)
    constexpr std::size_t fields_count = []() {

        if constexpr (concepts::aggregate_with_n_fields<T, indice>)
            return indice;
        else
            return fields_count<T, indice - 1>;
    }();

    template <typename from> requires std::is_lvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::tie(values...);
    }
    template <typename from> requires std::is_rvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::forward_as_tuple(std::move(values)...);
    }

	template <std::size_t N, concepts::aggregate T>
    struct element : std::type_identity<decltype([](){
		static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
	}())>{};

// GENERATED CONTENT, DO NOT EDIT MANUALLY !
#pragma region as_tuple_impl<N,T>
template <std::size_t N> requires (N == 1) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0 ] = value;
	return fwd_tie<decltype(value)>(v0);
}
template <std::size_t N> requires (N == 2) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1 ] = value;
	return fwd_tie<decltype(value)>(v0,v1);
}
template <std::size_t N> requires (N == 3) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2);
}
template <std::size_t N> requires (N == 4) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3);
}
template <std::size_t N> requires (N == 5) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4);
}
template <std::size_t N> requires (N == 6) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5);
}
template <std::size_t N> requires (N == 7) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6);
}
template <std::size_t N> requires (N == 8) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7);
}
template <std::size_t N> requires (N == 9) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8);
}
template <std::size_t N> requires (N == 10) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9);
}
template <std::size_t N> requires (N == 11) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10);
}
template <std::size_t N> requires (N == 12) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11);
}
template <std::size_t N> requires (N == 13) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12);
}
template <std::size_t N> requires (N == 14) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13);
}
template <std::size_t N> requires (N == 15) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14);
}
template <std::size_t N> requires (N == 16) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15);
}
template <std::size_t N> requires (N == 17) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16);
}
template <std::size_t N> requires (N == 18) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17);
}
template <std::size_t N> requires (N == 19) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18);
}
template <std::size_t N> requires (N == 20) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19);
}
template <std::size_t N> requires (N == 21) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20);
}
template <std::size_t N> requires (N == 22) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21);
}
template <std::size_t N> requires (N == 23) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22);
}
template <std::size_t N> requires (N == 24) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23);
}
template <std::size_t N> requires (N == 25) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24);
}
template <std::size_t N> requires (N == 26) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25);
}
template <std::size_t N> requires (N == 27) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26);
}
template <std::size_t N> requires (N == 28) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27);
}
template <std::size_t N> requires (N == 29) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28);
}
template <std::size_t N> requires (N == 30) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29);
}
template <std::size_t N> requires (N == 31) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30);
}
template <std::size_t N> requires (N == 32) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31);
}
template <std::size_t N> requires (N == 33) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32);
}
template <std::size_t N> requires (N == 34) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33);
}
template <std::size_t N> requires (N == 35) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34);
}
template <std::size_t N> requires (N == 36) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35);
}
template <std::size_t N> requires (N == 37) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36);
}
template <std::size_t N> requires (N == 38) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37);
}
template <std::size_t N> requires (N == 39) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38);
}
template <std::size_t N> requires (N == 40) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39);
}
template <std::size_t N> requires (N == 41) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40);
}
template <std::size_t N> requires (N == 42) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41);
}
template <std::size_t N> requires (N == 43) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42);
}
template <std::size_t N> requires (N == 44) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43);
}
template <std::size_t N> requires (N == 45) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44);
}
template <std::size_t N> requires (N == 46) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45);
}
template <std::size_t N> requires (N == 47) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46);
}
template <std::size_t N> requires (N == 48) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47);
}
template <std::size_t N> requires (N == 49) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48);
}
template <std::size_t N> requires (N == 50) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49);
}
template <std::size_t N> requires (N == 51) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50);
}
template <std::size_t N> requires (N == 52) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51);
}
template <std::size_t N> requires (N == 53) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52);
}
template <std::size_t N> requires (N == 54) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53);
}
template <std::size_t N> requires (N == 55) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54);
}
template <std::size_t N> requires (N == 56) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55);
}
template <std::size_t N> requires (N == 57) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56);
}
template <std::size_t N> requires (N == 58) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57);
}
template <std::size_t N> requires (N == 59) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58);
}
template <std::size_t N> requires (N == 60) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59);
}
template <std::size_t N> requires (N == 61) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60);
}
template <std::size_t N> requires (N == 62) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61);
}
template <std::size_t N> requires (N == 63) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62);
}
template <std::size_t N> requires (N == 64) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63);
}
template <std::size_t N> requires (N == 65) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64);
}
template <std::size_t N> requires (N == 66) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65);
}
template <std::size_t N> requires (N == 67) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66);
}
template <std::size_t N> requires (N == 68) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67);
}
template <std::size_t N> requires (N == 69) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68);
}
template <std::size_t N> requires (N == 70) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69);
}
template <std::size_t N> requires (N == 71) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70);
}
template <std::size_t N> requires (N == 72) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71);
}
template <std::size_t N> requires (N == 73) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72);
}
template <std::size_t N> requires (N == 74) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73);
}
template <std::size_t N> requires (N == 75) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74);
}
template <std::size_t N> requires (N == 76) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75);
}
template <std::size_t N> requires (N == 77) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76);
}
template <std::size_t N> requires (N == 78) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77);
}
template <std::size_t N> requires (N == 79) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78);
}
template <std::size_t N> requires (N == 80) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79);
}
template <std::size_t N> requires (N == 81) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80);
}
template <std::size_t N> requires (N == 82) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81);
}
template <std::size_t N> requires (N == 83) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82);
}
template <std::size_t N> requires (N == 84) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83);
}
template <std::size_t N> requires (N == 85) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84);
}
template <std::size_t N> requires (N == 86) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85);
}
template <std::size_t N> requires (N == 87) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86);
}
template <std::size_t N> requires (N == 88) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87);
}
template <std::size_t N> requires (N == 89) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88);
}
template <std::size_t N> requires (N == 90) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89);
}
template <std::size_t N> requires (N == 91) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90);
}
template <std::size_t N> requires (N == 92) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91);
}
template <std::size_t N> requires (N == 93) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92);
}
template <std::size_t N> requires (N == 94) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93);
}
template <std::size_t N> requires (N == 95) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94);
}
template <std::size_t N> requires (N == 96) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95);
}
template <std::size_t N> requires (N == 97) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96);
}
template <std::size_t N> requires (N == 98) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97);
}
template <std::size_t N> requires (N == 99) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98);
}
template <std::size_t N> requires (N == 100) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99);
}
template <std::size_t N> requires (N == 101) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100);
}
template <std::size_t N> requires (N == 102) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101);
}
template <std::size_t N> requires (N == 103) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102);
}
template <std::size_t N> requires (N == 104) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103);
}
template <std::size_t N> requires (N == 105) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104);
}
template <std::size_t N> requires (N == 106) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105);
}
template <std::size_t N> requires (N == 107) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106);
}
template <std::size_t N> requires (N == 108) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107);
}
template <std::size_t N> requires (N == 109) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108);
}
template <std::size_t N> requires (N == 110) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109);
}
template <std::size_t N> requires (N == 111) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110);
}
template <std::size_t N> requires (N == 112) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111);
}
template <std::size_t N> requires (N == 113) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112);
}
template <std::size_t N> requires (N == 114) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113);
}
template <std::size_t N> requires (N == 115) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114);
}
template <std::size_t N> requires (N == 116) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115);
}
template <std::size_t N> requires (N == 117) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116);
}
template <std::size_t N> requires (N == 118) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117);
}
template <std::size_t N> requires (N == 119) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118);
}
template <std::size_t N> requires (N == 120) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119);
}
template <std::size_t N> requires (N == 121) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120);
}
template <std::size_t N> requires (N == 122) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121);
}
template <std::size_t N> requires (N == 123) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122);
}
template <std::size_t N> requires (N == 124) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123);
}
template <std::size_t N> requires (N == 125) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124);
}
template <std::size_t N> requires (N == 126) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125);
}
template <std::size_t N> requires (N == 127) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126);
}
template <std::size_t N> requires (N == 128) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126,v127 ] = value;
	return fwd_tie<decltype(value)>(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126,v127);
}
#pragma endregion
#pragma region element<N, T>
	template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 1)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 2)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 3)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 4)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 5)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 6)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 7)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 8)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 9)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 10)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 11)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 12)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 13)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 14)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 15)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 16)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 17)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 18)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 19)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 20)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 21)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 22)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 23)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 24)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 25)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 26)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 27)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 28)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 29)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 30)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 31)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 32)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 33)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 34)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 35)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 36)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 37)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 38)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 39)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 40)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 41)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 42)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 43)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 44)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 45)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 46)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 47)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 48)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 49)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 50)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 51)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 52)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 53)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 54)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 55)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 56)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 57)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 58)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 59)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 60)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 61)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 62)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 63)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 64)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 65)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 66)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 67)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 68)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 69)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 70)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 71)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 72)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 73)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 74)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 75)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 76)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 77)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 78)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 79)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 80)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 81)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 82)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 83)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 84)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 85)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 86)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 87)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 88)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 89)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 90)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 91)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 92)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 93)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 94)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 95)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 96)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 97)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 98)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 99)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 100)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 101)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 102)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 103)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 104)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 105)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 106)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 107)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 108)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 109)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 110)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 111)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 112)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 113)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 114)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 115)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 116)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 117)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 118)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 119)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 120)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 121)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 122)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 123)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 124)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 125)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 126)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 127)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125),decltype(v126)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    template <std::size_t N, concepts::aggregate T>
    requires (fields_count<T> == 128)
    struct element<N, T> : decltype([]() -> decltype(auto) {
        auto && [ v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v84,v85,v86,v87,v88,v89,v90,v91,v92,v93,v94,v95,v96,v97,v98,v99,v100,v101,v102,v103,v104,v105,v106,v107,v108,v109,v110,v111,v112,v113,v114,v115,v116,v117,v118,v119,v120,v121,v122,v123,v124,v125,v126,v127 ] = declval<T&>();
        using tuple_type = std::tuple<decltype(v0),decltype(v1),decltype(v2),decltype(v3),decltype(v4),decltype(v5),decltype(v6),decltype(v7),decltype(v8),decltype(v9),decltype(v10),decltype(v11),decltype(v12),decltype(v13),decltype(v14),decltype(v15),decltype(v16),decltype(v17),decltype(v18),decltype(v19),decltype(v20),decltype(v21),decltype(v22),decltype(v23),decltype(v24),decltype(v25),decltype(v26),decltype(v27),decltype(v28),decltype(v29),decltype(v30),decltype(v31),decltype(v32),decltype(v33),decltype(v34),decltype(v35),decltype(v36),decltype(v37),decltype(v38),decltype(v39),decltype(v40),decltype(v41),decltype(v42),decltype(v43),decltype(v44),decltype(v45),decltype(v46),decltype(v47),decltype(v48),decltype(v49),decltype(v50),decltype(v51),decltype(v52),decltype(v53),decltype(v54),decltype(v55),decltype(v56),decltype(v57),decltype(v58),decltype(v59),decltype(v60),decltype(v61),decltype(v62),decltype(v63),decltype(v64),decltype(v65),decltype(v66),decltype(v67),decltype(v68),decltype(v69),decltype(v70),decltype(v71),decltype(v72),decltype(v73),decltype(v74),decltype(v75),decltype(v76),decltype(v77),decltype(v78),decltype(v79),decltype(v80),decltype(v81),decltype(v82),decltype(v83),decltype(v84),decltype(v85),decltype(v86),decltype(v87),decltype(v88),decltype(v89),decltype(v90),decltype(v91),decltype(v92),decltype(v93),decltype(v94),decltype(v95),decltype(v96),decltype(v97),decltype(v98),decltype(v99),decltype(v100),decltype(v101),decltype(v102),decltype(v103),decltype(v104),decltype(v105),decltype(v106),decltype(v107),decltype(v108),decltype(v109),decltype(v110),decltype(v111),decltype(v112),decltype(v113),decltype(v114),decltype(v115),decltype(v116),decltype(v117),decltype(v118),decltype(v119),decltype(v120),decltype(v121),decltype(v122),decltype(v123),decltype(v124),decltype(v125),decltype(v126),decltype(v127)>;
        return std::tuple_element<N, tuple_type>{};
    }()){};
    #pragma endregion
// END OF GENERATED CONTENT

    template <std::size_t>
    constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
        static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
    }
}
namespace csl::ag {

    constexpr auto as_tuple(auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::as_tuple_impl<details::fields_count<type>>(std::forward<decltype(value)>(value));
    }

	// tuple-like interface
	// size
    template <concepts::aggregate T>
    using size = std::integral_constant<std::size_t, details::fields_count<T>>;
	// get
    template <std::size_t N>
    constexpr decltype(auto) get(concepts::aggregate auto && value) {
        return ::std::get<N>(as_tuple(std::forward<decltype(value)>(value)));
    }
	// element
    template <std::size_t N, concepts::aggregate T>
    using element = typename details::element<N, T>;
}
