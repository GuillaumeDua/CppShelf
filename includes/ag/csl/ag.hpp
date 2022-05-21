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

    template <std::size_t>
    struct field_evaluator {
        explicit constexpr field_evaluator() = delete;

        template <typename T>
        constexpr operator T() const noexcept {
            // #ifdef __clang__
            return *((T*){ nullptr }); // for unevaluated context only
            // #else
            // return std::declval<T>(); // clang does not like it
            // #endif
            // return {};
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

    template <typename from> requires std::is_lvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::tie(values...);
    }
    template <typename from> requires std::is_rvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::forward_as_tuple(std::move(values)...);
    }

#pragma region as_tuple_impl<N,T>
// GENERATED CONTENT, DO NOT EDIT MANUALLY !
#pragma region CSL_AG_UNFOLD_IDENTITIES_WITH_N

    #define CSL_AG_UNFOLD_IDENTITIES_WITH_1(F) F(v0) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_2(F) F(v1), CSL_AG_UNFOLD_IDENTITIES_WITH_1(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_3(F) F(v2), CSL_AG_UNFOLD_IDENTITIES_WITH_2(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_4(F) F(v3), CSL_AG_UNFOLD_IDENTITIES_WITH_3(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_5(F) F(v4), CSL_AG_UNFOLD_IDENTITIES_WITH_4(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_6(F) F(v5), CSL_AG_UNFOLD_IDENTITIES_WITH_5(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_7(F) F(v6), CSL_AG_UNFOLD_IDENTITIES_WITH_6(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_8(F) F(v7), CSL_AG_UNFOLD_IDENTITIES_WITH_7(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_9(F) F(v8), CSL_AG_UNFOLD_IDENTITIES_WITH_8(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_10(F) F(v9), CSL_AG_UNFOLD_IDENTITIES_WITH_9(F) // NOLINT
#pragma endregion
#pragma region as_tuple_impl
     #define IDS(EXPR) EXPR
template <std::size_t N> requires (N == 1) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS));
}
template <std::size_t N> requires (N == 2) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS));
}
template <std::size_t N> requires (N == 3) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS));
}
template <std::size_t N> requires (N == 4) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS));
}
template <std::size_t N> requires (N == 5) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS));
}
template <std::size_t N> requires (N == 6) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS));
}
template <std::size_t N> requires (N == 7) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS));
}
template <std::size_t N> requires (N == 8) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS));
}
template <std::size_t N> requires (N == 9) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS));
}
template <std::size_t N> requires (N == 10) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS));
}
#undef F
     #pragma endregion
// END OF GENERATED CONTENT
#pragma endregion

    template <std::size_t>
    constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
        static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
    }
	template <std::size_t N, concepts::aggregate T>
    struct element : std::type_identity<decltype([](){
		static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
	}())>{};
}
namespace csl::ag {

    template <concepts::aggregate T, std::size_t indice = sizeof(T)>
    constexpr std::size_t fields_count = []() {
        static_assert(indice > 0);

        if constexpr (concepts::aggregate_with_n_fields<T, indice>)
            return indice;
        else
            return fields_count<T, indice - 1>;
    }();

    constexpr auto as_tuple(auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::as_tuple_impl<fields_count<type>>(std::forward<decltype(value)>(value));
    }

	// tuple-like
	// TODO : std::get/std::tuple_size/std::tuple_element
	// get
    template <std::size_t N>
    constexpr decltype(auto) get(concepts::aggregate auto && value) {
        return ::std::get<N>(as_tuple(std::forward<decltype(value)>(value)));
    }
	// size
    template <concepts::aggregate T>
    using size = std::integral_constant<std::size_t, fields_count<T>>;
	// element
    template <std::size_t N, concepts::aggregate T>
    using element = std::type_identity<decltype(get<N>(as_tuple(T{})))>;
}
