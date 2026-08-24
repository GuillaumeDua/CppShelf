#pragma once
// cpp shelf library : aggregates utility
// under MIT License - Copyright (c) 2021 Guillaume Dua "Guss"
// https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

// NOTE: Detectable by other csl libraries/headers (via #if defined(...)), even when not reachable through __has_include (e.g. Compiler Explorer raw-URL includes).
#define CSL_AG__INCLUDED

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

// TODO(Guillaume) constexpr inline function -> constexpr

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ag::details::unevaluated {
// for unevaluated context only

    template <typename T>
    consteval auto declval() noexcept -> std::add_rvalue_reference_t<T> {
        // static_assert([](){ return false; }(), "csl::ag::details : declval : for unevaluated context only !");
        if constexpr (std::is_lvalue_reference_v<T>)
            return *static_cast<std::remove_reference_t<T>*>(nullptr);
        else
            return std::move(*static_cast<std::remove_reference_t<T>*>(nullptr));
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
        constexpr static auto value = trait<bound_Ts..., Ts...>::value;
    };

    template <class, class>
    struct first_index_of;
    template <class T, typename... Ts>
    struct first_index_of<T, std::tuple<Ts...>> : std::integral_constant<std::size_t, 
        [](){
            static_assert(sizeof...(Ts), "first_index_of: requires a non-empty type sequence");
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
	//  not std::is_empty_v<T> and
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
}
namespace csl::ag::concepts::inline P2165 {
    // P2165 - tuple-like
    // Note that this is a good-enough implementation of P2165 to only fit this project's needs
	template <typename T, std::size_t N>
    concept tuple_element = requires(T t) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T>&>;
    };
    namespace details {
        // QUICK-FIX: Clang >= 18.1.8 Same mangled name error
        template <typename T>
        constexpr static auto valid_tuple_elements_v = []<std::size_t... I>(std::index_sequence<I...>) constexpr {
            return (true and ... and tuple_element<T, I>);
        }(std::make_index_sequence<std::tuple_size_v<T>>{});

        template <typename T>  // T must be unqualified
        concept unqualified_tuple_like =
            not std::is_reference_v<T>
            and requires {
                typename std::tuple_size<T>::type;
                requires std::same_as<std::remove_const_t<decltype(std::tuple_size_v<T>)>, std::size_t>;
            }
            and valid_tuple_elements_v<T>
        ;
    }
    template <typename T>
    concept tuple_like = details::unqualified_tuple_like<std::remove_cvref_t<T>>;
    template <typename T>
    concept pair_like = tuple_like<T> and std::tuple_size_v<std::remove_cvref_t<T>> == 2;
}
namespace csl::ag::concepts {
    template <typename T>
    concept range_like = requires(std::remove_cvref_t<T> & t) {
        t.begin();
        t.end();
    };

	template <typename T>
	concept structured_bindable = tuple_like<T> or aggregate<T>;
}

// generated : interface
namespace csl::ag::details::generated {
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

    template <std::size_t N> requires (N == 0) // NOLINT
        [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto &&) noexcept {
        return std::type_identity<std::tuple<>>{};
    }
    template <std::size_t N> requires (N == 0) // NOLINT
        [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto &&) noexcept {
        return std::tuple{};
    }
}

// --- generated: configuration ---
#if not defined(CSL_AG__USE_EMBEDDED_IMPLEMENTATION) and __has_include(<csl/ag/generated/configuration.hpp>)
#  include <csl/ag/generated/configuration.hpp>
#else
namespace csl::ag::configuration {
    constexpr static auto max_supported_fields_count = std::size_t{32};
}
#endif

namespace csl::ag::configuration {
    [[maybe_unused]] constexpr static auto is_bitfield_supported = 
    #if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
        true
    #else
        false
    #endif
    ;
}

# if defined(CSL_AG__VERBOSE_BUILD)
#   if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#     pragma message("csl::ag : CSL_AG__ENABLE_BITFIELDS_SUPPORT [enabled], slower algorithm selected")
#   else
#     pragma message("csl::ag : CSL_AG__ENABLE_BITFIELDS_SUPPORT [disabled], faster algorithm selected")
#   endif
# endif

// --- fields count probing ---
namespace csl::ag::details::probing {

    // Fast path: default_initializable T, no bitfield support.
    //  f(N) = aggregate_constructible_from_n_values<T,N> is monotone:
    //  - true:  all N in [0, field_count]
    //  - false: all N > field_count
    //  O(log field_count) instantiations via ascending exponential probe + binary search.
    template <concepts::aggregate T>
    requires (std::default_initializable<T>)
    struct fast_path {

        // Phase 2: binary search in (lower_limit, higher_limit].
        //  invariant: f(N) is monotone so f(lower_limit)=true, f(higher_limit)=false.
        template <std::size_t lower_limit, std::size_t higher_limit>
        [[nodiscard]] consteval static auto bisect() noexcept -> std::size_t {

            if constexpr (lower_limit + 1 == higher_limit)
                return lower_limit;
            else {
                constexpr auto mid = lower_limit + ((higher_limit - lower_limit) / 2);
                if constexpr (concepts::aggregate_constructible_from_n_values<T, mid>)
                    return bisect<mid, higher_limit>();
                else
                    return bisect<lower_limit, mid>();
            }
        }

        // Phase 1: exponential probe, lower_limit is the last known true.
        //  Doubles until f(2*lower_limit)=false or cap is exceeded, then hands off to bisect.
        template <std::size_t lower_limit>
        requires (lower_limit > configuration::max_supported_fields_count)
        [[nodiscard]] consteval static auto probe() noexcept -> std::size_t {
            static_assert(lower_limit <= configuration::max_supported_fields_count,
                "[csl::ag] fields_count: cannot determine T's field count. "
                "The type likely has more fields than csl::ag::configuration::max_supported_fields_count."
            );
            return {};
        }
        template <std::size_t lower_limit>
        [[nodiscard]] consteval static auto probe() noexcept -> std::size_t {

            constexpr std::size_t higher_limit = lower_limit * 2;
            if constexpr (
                higher_limit > configuration::max_supported_fields_count
                or not concepts::aggregate_constructible_from_n_values<T, higher_limit>
            )
                return bisect<lower_limit, higher_limit>();
            else
                return probe<higher_limit>();
        }
    };

    // Slow path: non-default_initializable T, or bitfield support enabled.
    //  f(N) is non-monotone for non-default_initializable T, so binary search would be unsafe.
    //  Linear descent from an upper bound > field_count is the only safe approach.
    template <concepts::aggregate T>
    struct slow_path {

        static_assert(not std::is_reference_v<T>);
        static_assert(not std::is_empty_v<T>);

        template <std::size_t field_detection_indice>
        requires (field_detection_indice == 0)
        [[nodiscard]] consteval static auto probe() noexcept -> std::size_t {
            static_assert(field_detection_indice not_eq 0,
                "[csl::ag] fields_count: cannot determine T's field count. "
                "The type likely has more fields than csl::ag::configuration::max_supported_fields_count."
            );
            return {};
        }
        template <std::size_t field_detection_indice>
        [[nodiscard]] consteval static auto probe() noexcept -> std::size_t {
            if constexpr (concepts::aggregate_constructible_from_n_values<T, field_detection_indice>)
                return field_detection_indice;
            else
                return probe<field_detection_indice - 1>();
        }
    };

    template <concepts::aggregate T>
    constexpr inline static std::size_t fields_count = []() consteval -> std::size_t {
#if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT)
        // NOTE: upper_bound can exceed max_supported_fields_count even when the actual field count does not
        // (e.g. a 4-byte struct with 3 bitfields has upper_bound=8 but field_count=3),
        // so no static_assert here: slow_path::probe<0> is responsible to handle the error when the cap exceeds.
        constexpr std::size_t upper_bound = sizeof(T) / alignof(T) * sizeof(std::byte) * CHAR_BIT;
        return slow_path<T>::template probe<std::min(upper_bound, configuration::max_supported_fields_count)>();
#else
        if constexpr (std::default_initializable<T>)
            return fast_path<T>::template probe<1>();
        else {
            constexpr std::size_t upper_bound = sizeof(T) / alignof(T);
            if constexpr (upper_bound > configuration::max_supported_fields_count)
                static_assert(upper_bound <= configuration::max_supported_fields_count,
                    "[csl::ag] fields_count: sizeof(T)/alignof(T) exceeds csl::ag::configuration::max_supported_fields_count. "
                    "Increase CSL_AG__MAX_SUPPORTED_FIELDS_COUNT when building with CMake."
                );
            else
                return slow_path<T>::template probe<upper_bound>();
        }
#endif
    }();
    template <concepts::aggregate T>
    requires std::is_empty_v<T>
    constexpr inline static std::size_t fields_count<T> = 0;
}

// --- tuple adapter ---
namespace csl::ag::details {

    // Declaration only: body defined after generated specializations,
    // so the non-dependent qualified lookup of details::generated::make_to_tuple<size> sees all N variants.
    [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value)
    // -> std::type_identity<std::tuple<field_Ts...>>
    ;

    template <typename T>
    using to_tuple_t = mp::copy_cvref_t<
        T,
        typename std::remove_cvref_t<decltype(csl::ag::details::make_to_tuple(std::declval<std::remove_cvref_t<T>>()))>::type
    >;

    template <typename owner_type>
    [[nodiscard]] constexpr static concepts::tuple_like auto make_tuple_view(auto && ... values) noexcept {
        using tuple_t = to_tuple_t<std::remove_cvref_t<owner_type>>;

        constexpr auto size = std::tuple_size_v<tuple_t>;
        static_assert(size == sizeof...(values));
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return std::forward_as_tuple(
                static_cast<mp::field_view_t<owner_type, std::tuple_element_t<indexes, tuple_t>>>(values)...
            );
        }(std::make_index_sequence<size>{});
    }
}

// --- generated: implementations ---
#if not defined(CSL_AG__USE_EMBEDDED_IMPLEMENTATION) and __has_include(<csl/ag/generated/to_tuple.hpp>)
#  include <csl/ag/generated/to_tuple.hpp>
#else
namespace csl::ag::details::generated {
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
#endif

// --- tuple adapter: make_to_tuple definition ---
// Defined here so `details::generated::make_to_tuple<size>` (non-dependent qualified name) resolves against all generated specializations above, not just the base template.
namespace csl::ag::details {
    [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value)
    // -> std::type_identity<std::tuple<field_Ts...>>
    {
        constexpr auto size = details::probing::fields_count<std::remove_cvref_t<decltype(value)>>;
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
    struct size : std::integral_constant<std::size_t, details::probing::fields_count<std::remove_reference_t<T>>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr inline static auto size_v = size<T>::value;

    // empty
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
    //
    //  factory that creates a lightweight accessor to an existing aggregate value,
    //  extending owner's value-semantic to owned values,
    //  while preserving value-semantic of ref-qualified values
    //
    //  ex:
    //  - struct type{ A v0; B & v1; const C && v2 }
    //  -       type &  => std::tuple<      A&,        B&, const C&&>;
    //  - const type &  => std::tuple<const A&,  const B&, const C&&>;
    //  -       type && => std::tuple<      A&&,       B&, const C&&>;
    [[nodiscard]] constexpr auto to_tuple_view(concepts::aggregate auto && value) noexcept {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::generated::to_tuple_view_impl<details::probing::fields_count<type>>(std::forward<decltype(value)>(value));
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
    struct tuple_size : std::integral_constant<std::size_t, details::probing::fields_count<std::remove_reference_t<T>>>{};
	template <csl::ag::concepts::aggregate T>
	constexpr inline static auto tuple_size_v = tuple_size<T>::value;

    // tuple_element
    template <std::size_t N, concepts::aggregate T>
    using tuple_element = std::tuple_element<N, details::to_tuple_t<std::remove_cvref_t<T>>>;
	template <std::size_t N, concepts::aggregate T>
	using tuple_element_t = typename tuple_element<N, T>::type;

    // get<std::size_t>
    template <std::size_t N>
    [[nodiscard]] constexpr decltype(auto) get(concepts::aggregate auto && value)
    noexcept
    requires (N < size_v<std::remove_cvref_t<decltype(value)>>)
    {
        return ::std::get<N>(to_tuple_view(std::forward<decltype(value)>(value)));
    }
    // get<T>
    template <typename T>
    [[nodiscard]] constexpr decltype(auto) get(concepts::aggregate auto && value)
    noexcept
    requires (0 not_eq size_v<std::remove_cvref_t<decltype(value)>>)
    {
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
    //  interally performs get<indexes>...
    // WARNING: if csl::ag::size_v<decltype(from_value)> is less than the amount of elements
    //  required to perform an aggregate initialization of T,
    //  which will results in some uninitialized fields,
    //  effectively producing `-Wmissing-field-initializers`, just like std::make_from_tuple`.
    //
    // REFACTO: universal template
    // REFACTO: use apply
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
    // transformation/conversion factory. unfold into an either complete or template type T
    // interally performs get<Ts>... (requires unique<Ts...>)
    // motivation: struct { int; string } => struct { string; int } with to_string/from_chars
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
    constexpr static auto to(){ return to_complete_type_tag<T>{}; };
    template <template <typename...> typename T>
    constexpr static auto to(){ return to_template_type_ttps_tag<T>{}; };
    template <template <typename, auto...> typename T> 
    constexpr static auto to(){ return to_template_type_ttp_nttps_tag<T>{}; };
    template <template <auto, typename ...> typename T>
    constexpr static auto to(){ return to_template_type_nttp_ttps_tag<T>{}; };

    template <typename T>
    [[nodiscard]] constexpr static auto operator|(csl::ag::concepts::aggregate auto && value, to_complete_type_tag<T>)
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

    [[maybe_unused]] constexpr inline static auto all = all_view_tag{};
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
        concepts::aggregate<T>
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

// --- universal API ---
// homogeneous API for tuple-likes and csl::ag::concepts::aggregates
//
// [Design] WHY this does NOT specialize std::tuple_size / std::tuple_element for non-STL aggregates:
//
//  The generated code (included at line ~400 from generated/to_tuple.hpp) defines
//  make_to_tuple<N> using structured bindings:
//      auto && [ v0, v1, ... ] = value;
//  C++ selects the binding strategy at template instantiation time:
//    - std::tuple_size<T> absent  ->  aggregate-field binding  (no get<I> needed)
//    - std::tuple_size<T> present ->  tuple-like binding, which calls get<I>(value)
//                                     via unqualified lookup + ADL
//
//  If std::tuple_size<T> were specialized for non-STL aggregates,
//  then the generated make_to_tuple<N> templates would require get<I>(value) to be ADL-findable for T.
//  csl::ag::get is defined AFTER the generated include, so it is not visible via non-ADL unqualified lookup at the template definition site.
//  For user-defined aggregate types outside namespace csl::ag, ADL does not search csl::ag either - making get<I> irrecoverably unfindable for them.
//
//  The generated code exists because C++ has no introspection for aggregate field types:
//  make_to_tuple<N> uses structured bindings at consteval time to capture field types as a std::tuple via decltype,
//  producing the std::type_identity<std::tuple<Ts...>> that drives csl::ag::element<I, T> and csl::ag::to_tuple_t<T>.

namespace csl::ag::tuplelike::concepts {
    template <typename T>
    concept non_stl_aggregate = csl::ag::concepts::aggregate<T>
        and not csl::ag::concepts::tuple_like<T>
    ;
}
namespace csl::ag::tuplelike {

    // size
    template <typename T>
    struct size;
    template <concepts::non_stl_aggregate T>
    struct size<T> : csl::ag::size<T>{};
    template <csl::ag::concepts::tuple_like T>
    struct size<T> : std::tuple_size<T>{};
    template <typename T>
    constexpr inline static auto size_v = size<T>::value;

    // element
    template <std::size_t, typename>
    struct element;
    template <std::size_t I, concepts::non_stl_aggregate T>
    struct element<I, T> : csl::ag::element<I, T>{};
    template <std::size_t I, csl::ag::concepts::tuple_like T>
    struct element<I, T> : std::tuple_element<I, T>{};
    template <std::size_t I, typename T>
    using element_t = typename element<I, T>::type;

    // get
    template <std::size_t index>
    constexpr auto get(concepts::non_stl_aggregate auto && value) -> decltype(auto) {
        return csl::ag::get<index>(csl_fwd(value));
    }
    template <std::size_t index>
    constexpr auto get(csl::ag::concepts::tuple_like auto && value) -> decltype(auto) {
        using std::get;
        return get<index>(csl_fwd(value)); // ADL
    }

    // algorithms
    // - apply
    constexpr auto apply(auto && f, csl::ag::concepts::structured_bindable auto && value) -> decltype(auto) {
        using value_type = std::remove_cvref_t<decltype(value)>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr -> decltype(auto) {
            return std::invoke(csl_fwd(f), csl::ag::tuplelike::get<indexes>(csl_fwd(value))...);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<value_type>>{});
    }
    // - for_each
    constexpr auto for_each(csl::ag::concepts::structured_bindable auto && value, auto && f) -> decltype(auto) {
        using value_type = std::remove_cvref_t<decltype(value)>;
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            ((
                std::invoke(csl_fwd(f), csl::ag::tuplelike::get<indexes>(csl_fwd(value)))
            ), ...);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<value_type>>{});
    }
    // - for_each_enumerated
    constexpr auto for_each_enumerated(csl::ag::concepts::structured_bindable auto && value, auto && f) -> decltype(auto) {
        using value_type = std::remove_cvref_t<decltype(value)>;
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            ((
                csl_fwd(f).template operator()<indexes>(
                    csl::ag::tuplelike::get<indexes>(csl_fwd(value))
                )
            ), ...);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<value_type>>{});
    }
    // - for_each_zipped
    template <typename ... Ts>
    requires (true and ... and csl::ag::concepts::structured_bindable<Ts>)
    constexpr void for_each_zipped(auto && f, Ts &&... values) {
        constexpr std::size_t min_size = std::min({size_v<std::remove_reference_t<Ts>>...});

        const auto invoke_at_index = [&]<std::size_t index>() constexpr {
            f(get<index>(std::forward<decltype(values)>(values))...);
        };

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            (
                invoke_at_index.template operator()<indexes>()
            , ...);
        }(std::make_index_sequence<min_size>{});
    }
}

// ------------------------------
//  formatting/printing - core
// ------------------------------
//
// csl/ag.hpp only ships the backend-agnostic formatting core (std-only, always compiled, no blanket specialization):
// - format options
// - format tags
// - decorated views
// - view operator|
// - type_name CPO (customization trait)
// - formatters shared machinery.
//
// Formatting backends are opt-in feature headers, include them consistently program-wide:
//
//  <csl/ag/formatting/backend/std_format.hpp>  std::formatter support
//  <csl/ag/formatting/backend/fmt.hpp>         fmt::formatter support (fmtlib is provided by the consumer)
//  <csl/ag/formatting/backend/ostream.hpp>     operator<<(std::ostream &, ...) support
//  <csl/ag/formatting/typeinfo.hpp>            compile-time type names for the typenamed option (csl::typeinfo-backed)

// type_name
#pragma region type_name

#include <typeindex>

namespace csl::ag::formatting {

    /// @brief Customization point (type-trait) providing the type name written by the @c typenamed formatting option.
    ///
    /// The primary template is a runtime fallback: @c std::type_index(typeid(T)).name(), which is implementation-defined and possibly mangled.
    ///
    /// To obtain readable names, either:
    /// - @c \#include @c <csl/ag/formatting/typeinfo.hpp> for compile-time, demangled names (csl::typeinfo-backed),
    /// - or specialize this trait for specific types/concepts/constraints.
    ///
    /// @tparam T the type whose name is queried.
    template <typename T>
    struct type_name {
        static inline const std::string_view value = std::type_index(typeid(T)).name();
    };
    
    /// @brief Alias for @c type_name<T>::value, the type name written by the @c typenamed option.
    ///
    /// @note Deliberately a @c constexpr reference, not a copy.
    /// Per @c [basic.start.dynamic], instantiated specializations have @e unordered dynamic initialization;
    /// a local copy of @c type_name<T>::value could be initialized before its source and capture an empty @c string_view.
    //
    /// The binding is an address constant expression even for the runtime primary template, so @c type_name_v is guaranteed constant-initialized,
    /// and usable in constant expressions whenever the specialization's @c value is @c constexpr (e.g. @c <csl/ag/formatting/typeinfo.hpp>).
    ///
    /// @tparam T the type whose name is queried.
    template <typename T>
    constexpr static inline const std::string_view & type_name_v = type_name<T>::value;
}

#pragma endregion

// Formatting(shared): composable format options, tag types, decorated view, operator|

namespace csl::ag::formatting {
    /// \brief Bitmask of composable formatting options.
    ///        NOTE: underlying type is long for compatibility with std::ios_base::iword().
    enum class format_options : long { // NOLINT(*-enum-size, *-runtime-int)
        none      = 0,
        no_braces = 1L << 0,
        indented  = 1L << 1,
        indexed   = 1L << 2,
        typenamed = 1L << 3,
    };

    [[nodiscard]] constexpr auto operator|(format_options a, format_options b) noexcept -> format_options {
        return format_options{ std::to_underlying(a) | std::to_underlying(b) };
    }
    [[nodiscard]] constexpr auto operator&(format_options a, format_options b) noexcept -> format_options {
        return format_options{ std::to_underlying(a) & std::to_underlying(b) };
    }
    [[nodiscard]] constexpr auto operator~(format_options a) noexcept -> format_options {
        return format_options{ ~std::to_underlying(a) };
    }
    constexpr auto operator|=(format_options & a, format_options b) noexcept -> format_options & {
        return (a = a | b);
    }

    /// \brief Formatting option tags.
    /// Implicitly convertible to format_options
    struct indented_t {
        constexpr static format_options value = format_options::indented;
        constexpr operator format_options() const noexcept { return value; } // NOLINT(*-explicit-constructor)
    };
    struct no_braces_t {
        constexpr static format_options value = format_options::no_braces;
        constexpr operator format_options() const noexcept { return value; } // NOLINT(*-explicit-constructor)
    };
    struct indexed_t {
        constexpr static format_options value = format_options::indexed;
        constexpr operator format_options() const noexcept { return value; } // NOLINT(*-explicit-constructor)
    };
    struct typenamed_t {
        constexpr static format_options value = format_options::typenamed;
        constexpr operator format_options() const noexcept { return value; } // NOLINT(*-explicit-constructor)
    };

    [[maybe_unused]] constexpr inline static indented_t  indented{};
    [[maybe_unused]] constexpr inline static no_braces_t no_braces{};
    [[maybe_unused]] constexpr inline static indexed_t   indexed{};
    [[maybe_unused]] constexpr inline static typenamed_t typenamed{};

    namespace details {
        constexpr inline format_options all_format_options_mask =
            format_options::no_braces
            | format_options::indented
            | format_options::indexed
            | format_options::typenamed
        ;
    }

    namespace concepts {
        template <typename T>
        concept format_option =
            requires { { T::value } -> std::same_as<const format_options &>; }
            and (T::value != format_options::none)
            and ((T::value & details::all_format_options_mask) == T::value)
        ;
    }

    /// \brief Combine two tags into a format_options bitmask, e.g. `indented | typenamed`.
    template <concepts::format_option Lhs, concepts::format_option Rhs>
    [[nodiscard]] constexpr auto operator|(Lhs, Rhs) noexcept -> format_options {
        return Lhs::value | Rhs::value;
    }
    /// \brief Extend an accumulated bitmask with one more tag, e.g. `(indented | typenamed) | indexed`.
    template <concepts::format_option Option>
    [[nodiscard]] constexpr auto operator|(format_options a, Option) noexcept -> format_options {
        return a | Option::value;
    }
    template <concepts::format_option Option>
    [[nodiscard]] constexpr auto operator|(Option, format_options b) noexcept -> format_options {
        return Option::value | b;
    }
}

namespace csl::ag::formatting::details::decorators {

    /// \brief Carries formatting informations.
    template <typename T>
    struct formatted_view_t {

        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "requires unqualified T");

        using csl_ag_formatting_decorator = void;
        using value_type = T;

        /*explicit*/ operator const value_type &() const { return value; } // NOLINT(*-explicit-constructor)
        const value_type & value; // NOLINT(*-non-private-member-variables-in-classes, *-avoid-const-or-ref-data-members)

        format_options options = format_options::none;
        std::size_t    depth   = 0;
    };
}

namespace csl::ag::formatting::details::concepts {
    // NOTE: type-requirement (typename): csl_ag_formatting_decorator is a member type alias -
    //       a simple-requirement would test it as an (invalid) expression and never be satisfied
    template <typename T>
    concept decorator = requires { typename T::csl_ag_formatting_decorator; };
}

namespace csl::ag::formatting {

    /// \brief structured_bindable T | format_options (one tag, e.g. `indented`, or several composed, e.g. `indented | indexed`) => formatted_view_t
    /// Tags (indented, indexed, typenamed, no_braces) implicitly convert to format_options, so this single overload covers both cases.
    template <csl::ag::concepts::structured_bindable T>
    requires (not concepts::format_option<T>)
    [[nodiscard]] auto operator|(const T & value, format_options options)
    -> details::decorators::formatted_view_t<std::remove_cvref_t<T>>
    {
        return {
            .value = value,
            .options = options
        };
    }

    /// \brief formatted_view_t | composed format_options => accumulated view (same depth)
    template <typename T>
    [[nodiscard]] auto operator|(details::decorators::formatted_view_t<T> view, format_options options)
    -> details::decorators::formatted_view_t<T>
    {
        return {
            .value = view.value,
            .options = view.options | options,
            .depth = view.depth
        };
    }
}

namespace csl::ag::formatting::concepts {

    /// \brief Supported formatting character types
    ///
    /// \note Consistent with the standard  [format.formatter.spec]: only char and wchar_t, not so char8_t/char16_t/char32_t
    ///       Should the standard grow support of charN_t family, then this concept will change.
    template <typename CharT>
    concept supported_char_type
        =   std::same_as<CharT, char>
        or  std::same_as<CharT, wchar_t>
    ;
}

/// \brief Narrow-to-CharT conversions, and compile-time storage for narrow string literals.
namespace csl::ag::formatting::details::widening {

    /// \brief The conversion policy every backend applies to narrow content: type names, presentation literals, char fields.
    template <typename CharT>
    [[nodiscard]] constexpr auto widen(char value) noexcept -> CharT {
        if constexpr (std::same_as<CharT, char>)
            return value;
        else
            return static_cast<CharT>(static_cast<unsigned char>(value));
    }

    /// \brief Structural NTTP wrapper for a narrow string literal, null-terminator included.
    template <std::size_t N>
    struct fixed_string {
        std::array<char, N> value;

        // NOLINTNEXTLINE(*-avoid-c-arrays,*-explicit-constructor,*-explicit-conversions)
        consteval fixed_string(const char (&literal)[N]) {
            
            if constexpr (N == 0)
                throw std::invalid_argument{ "[csl::ag::formatting] fixed_string expects an input size > 0" };
            if (literal[N - 1] != '\0')
                throw std::invalid_argument{ "[csl::ag::formatting] fixed_string expects a null-terminated string literal" };

            for (std::size_t i = 0; i < N; ++i)
                value[i] = literal[i]; // NOLINT(*-pro-bounds-constant-array-index)
        }
    };

    /// \brief Storage for a narrow literal widened to CharT.
    template <typename CharT, fixed_string literal>
    constexpr static inline auto literal_storage = [] {
        std::array<CharT, literal.value.size()> result{};
        std::ranges::transform(literal.value, result.begin(), widen<CharT>);
        return result;
    }();

    /// \brief A narrow string literal as a basic_string_view<CharT>, e.g. literal_v<CharT, ", ">, excluding terminators
    template <typename CharT, fixed_string literal>
    constexpr static inline std::basic_string_view<CharT> literal_v {
        literal_storage<CharT, literal>.data(),
        literal.value.size() - 1
    };
}

/// \brief Formatting presentation constants/helpers shared by every backend (std::ostream, fmt, std::format).
namespace csl::ag::formatting::details::style {

    constexpr static std::size_t indentation_width = 4;

    template <typename T, typename CharT = char>
    [[nodiscard]] constexpr auto opening_bracket() noexcept -> std::basic_string_view<CharT> {
        if constexpr (csl::ag::concepts::range_like<T>)      return widening::literal_v<CharT, "[">;
        else if constexpr (csl::ag::concepts::tuple_like<T>) return widening::literal_v<CharT, "(">;
        else                                                 return widening::literal_v<CharT, "{">;
    }
    template <typename T, typename CharT = char>
    [[nodiscard]] constexpr auto closing_bracket() noexcept -> std::basic_string_view<CharT> {
        if constexpr (csl::ag::concepts::range_like<T>)      return widening::literal_v<CharT, "]">;
        else if constexpr (csl::ag::concepts::tuple_like<T>) return widening::literal_v<CharT, ")">;
        else                                                 return widening::literal_v<CharT, "}">;
    }

    /// \brief Brackets/separator for one formatted node, resolved once from options.
    template <typename CharT>
    struct brackets_t {
        std::basic_string_view<CharT> opening_bracket;
        std::basic_string_view<CharT> closing_bracket;
        std::basic_string_view<CharT> separator;
    };
    template <typename T, typename CharT>
    [[nodiscard]] constexpr auto make_brackets(format_options options) noexcept -> brackets_t<CharT> {

        if (bool(options & format_options::no_braces))
            return {};
        return {
            .opening_bracket = opening_bracket<T, CharT>(),
            .closing_bracket = closing_bracket<T, CharT>(),
            .separator       = bool(options & format_options::indented)
                ? widening::literal_v<CharT, ",">
                : widening::literal_v<CharT, ", ">
        };
    }
}

// Formatters shared machinery (fmt::formatter / std::formatter): backend-agnostic, std-only.

namespace csl::ag::formatting::details {

    template <typename CharT, std::size_t N>
    [[nodiscard]] consteval auto to_chars() noexcept {

        constexpr auto digits = [] {
            std::size_t n = N;
            std::size_t count = 1;
            while (n >= 10) { n /= 10; ++count; }
            return count;
        }();

        std::array<CharT, digits> result{};
        auto index = N;
        for (std::size_t i = digits; i > 0; --i) {
            result[i - 1] = CharT{'0'} + static_cast<CharT>(index % 10);
            index /= 10;
        }
        return result;
    }

    template <typename CharT, typename OutputIt>
    [[nodiscard]] auto write(OutputIt out, std::basic_string_view<CharT> sv) noexcept -> OutputIt {
        for (CharT c : sv)
            *out++ = c;
        return out;
    }

    template <typename CharT, typename OutputIt>
    [[nodiscard]] auto write(OutputIt out, CharT c) noexcept -> OutputIt
    requires requires { *out = c; }
    {
        *out = c;
        return ++out;
    }

    /// \brief Write a narrow string into a CharT output, applying the widening policy.
    ///        Used for type names: the compiler-provided sources (__PRETTY_FUNCTION__, std::type_index::name()) are narrow by construction
    template <typename CharT, typename OutputIt>
    [[nodiscard]] auto write_narrow(OutputIt out, std::string_view value) noexcept -> OutputIt {
        if constexpr (std::same_as<CharT, char>)
            return write<CharT>(out, value);
        else
            return std::ranges::transform(value, out, widening::widen<CharT>).out;
    }

    /// \brief Maps a formatter_implementation (fmt::formatter or std::formatter) to its matching format-error exception type
    ///        Note that the 3rd ttp of fmt::formatter is defaulted (SFINAE, void)
    template <template <typename, typename> class formatter_implementation>
    struct format_error_type;
    template <template <typename, typename> class formatter_implementation>
    using format_error_type_t = typename format_error_type<formatter_implementation>::type;

    /// \brief Formatters shared logic (fmt and std) -> parse, format.
    ///        Format options/depth are carried at runtime by the formatted_view_t decorator (see operator|), not as TTPs.
    ///        parse() additionally accumulates a parse_options mask from the format-spec string (e.g. ':n'),
    ///        merged with the decorator's options for this node only - it is not propagated to field formatters.
    /// \tparam formatter_implementation fmt::formatter or std::formatter (yet, unconstrained here)
    /// \tparam T the structured-bindable aggregate type being formatted
    /// \tparam CharT the character type
    template <
        template <typename, typename> class formatter_implementation,
        csl::ag::concepts::structured_bindable T,
        csl::ag::formatting::concepts::supported_char_type CharT
    >
    requires (not details::concepts::decorator<T>)
    class ag_formatter_base {

        template <std::size_t ... indexes>
        static auto make_field_formatters(std::index_sequence<indexes...>)
        -> std::tuple<
            formatter_implementation<
                decorators::formatted_view_t<csl::ag::tuplelike::element_t<indexes, T>>,
                CharT
            >...
        >;
        using field_formatters_t = decltype(make_field_formatters(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{}));
        field_formatters_t field_formatters{};

        format_options parse_options = format_options::none; // mask, possibly changed with parse() (e.g. :n, etc.)

        template <std::size_t FieldIndex, typename FieldType, typename Context>
        void format_element(
            const FieldType & field_value,
            Context & context,
            std::basic_string_view<CharT> separator,
            format_options options,
            std::size_t depth
        ) const {

            const auto indented = bool(options & format_options::indented);

            if constexpr (FieldIndex > 0) {
                context.advance_to(write<CharT>(context.out(), separator));
                if (indented)
                    context.advance_to(write<CharT>(context.out(), '\n'));
            }

            if (indented)
                context.advance_to(std::fill_n(context.out(), (depth + 1) * style::indentation_width, CharT{' '}));

            if (bool(options & format_options::indexed)) {
                context.advance_to(write<CharT>(context.out(), '['));
                constexpr auto index_chars = to_chars<CharT, FieldIndex>();
                context.advance_to(write<CharT>(context.out(), std::basic_string_view<CharT>{index_chars.data(), index_chars.size()}));
                context.advance_to(write<CharT>(context.out(), ']'));
                context.advance_to(write<CharT>(context.out(), ' '));
            }

            if (bool(options & format_options::typenamed)) {
                context.advance_to(write_narrow<CharT>(context.out(), type_name_v<FieldType>));
                context.advance_to(write<CharT>(context.out(), ':'));
                context.advance_to(write<CharT>(context.out(), ' '));
            }

            decorators::formatted_view_t<FieldType> field_view{
                .value   = field_value,
                .options = options & ~format_options::no_braces, // keep everything but no_braces (outermost-only)
                .depth   = depth + 1
            };
            context.advance_to(std::get<FieldIndex>(field_formatters).format(field_view, context));
        }

        template <typename Context>
        auto do_format(const T & value, format_options options, std::size_t depth, Context & context) const {

            const format_options effective_options = options | parse_options;
            const bool indented = bool(effective_options & format_options::indented);
            const auto brackets = style::make_brackets<T, CharT>(effective_options);

            context.advance_to(write<CharT>(context.out(), brackets.opening_bracket));
            if (indented)
                context.advance_to(write<CharT>(context.out(), '\n'));

            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
                (
                    format_element<indexes>(
                        csl::ag::tuplelike::get<indexes>(value),
                        context,
                        brackets.separator,
                        effective_options,
                        depth
                    )
                , ...);
            }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});

            if (indented) {
                context.advance_to(write<CharT>(context.out(), '\n'));
                context.advance_to(std::fill_n(context.out(), depth * style::indentation_width, CharT{' '}));
            }
            context.advance_to(write<CharT>(context.out(), brackets.closing_bracket));
            return context.out();
        }

    public:
        using csl_ag_product = void;

        /// \brief Format-spec letters (runtime overrides, accumulated into parse_options):
        ///     n = no_braces
        ///     i = indented
        ///     x = indexed
        ///     t = typenamed.
        ///     Unrecognized letters result in format_error
        constexpr auto parse(auto & context) {

            auto it  = context.begin();
            auto end = context.end();
            while (it != end and *it != static_cast<CharT>('}')) {
                switch (static_cast<char>(*it)) {
                    case 'n': parse_options |= format_options::no_braces; break;
                    case 'i': parse_options |= format_options::indented;  break;
                    case 'x': parse_options |= format_options::indexed;   break;
                    case 't': parse_options |= format_options::typenamed; break;
                    default: throw format_error_type_t<formatter_implementation>{"csl::ag::formatting: unrecognized format-spec letter (expected one of: n, i, x, t)"};
                }
                ++it;
            }
            context.advance_to(it);

            // NOTE: Propagate parse to field formatters with an empty spec
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
                ([&] {
                    auto empty_context = std::remove_cvref_t<decltype(context)>(std::basic_string_view<CharT>{});
                    std::get<indexes>(field_formatters).parse(empty_context);
                }(), ...);
            }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});

            return it;
        }

        template <typename Context>
        auto format(const T & value, Context & context) const {
            return do_format(value, format_options::none, 0, context);
        }
        template <typename Context>
        auto format(const decorators::formatted_view_t<T> & view, Context & context) const {
            return do_format(view.value, view.options, view.depth, context);
        }
    };

    /// \brief Support non-structured-bindable field types.
    /// Only formats the value: the parent already wrote indentation + format-options prefixes (indexed, typenamed).
    template <
        template <typename, typename> class formatter_implementation,
        typename T,
        csl::ag::formatting::concepts::supported_char_type CharT
    >
    requires (not csl::ag::concepts::structured_bindable<T>)
    class ag_formatter_base_leaf {

        // NOTE: fmt provides no formatter<char, wchar_t> : it maps a narrow char to the target character type instead.
        using leaf_type = std::conditional_t<std::same_as<T, char>, CharT, T>;

        [[nodiscard]] constexpr static auto as_leaf(const T & value) noexcept -> decltype(auto) {
            if constexpr (std::same_as<leaf_type, T>)
                return (value);
            else
                return widening::widen<leaf_type>(value);
        }

        formatter_implementation<leaf_type, CharT> value_formatter{};
    public:
        using csl_ag_product = void;

        constexpr auto parse(auto & context) {
            auto empty_context = std::remove_cvref_t<decltype(context)>(std::basic_string_view<CharT>{});
            value_formatter.parse(empty_context);
            return context.begin();
        }
        /// \brief Format, quoted
        /// NOTE: `formatter.set_debug_format()` is implemented on GCC-13's libstdc++, or might depend on __cpp_lib_format_ranges
        template <typename Context>
        auto format(const T & value, Context & context) const {

            if constexpr (requires { value_formatter.set_debug_format(); }){
                value_formatter.set_debug_format();
                return value_formatter.format(as_leaf(value), context);
            }
            else if constexpr (std::same_as<T, CharT> or std::same_as<T, char>) {
                context.advance_to(write<CharT>(context.out(), static_cast<CharT>('\'')));
                context.advance_to(value_formatter.format(as_leaf(value), context));
                return write<CharT>(context.out(), static_cast<CharT>('\''));
            }
            else if constexpr (std::convertible_to<T, std::basic_string_view<CharT>>) {
                auto out = write<CharT>(context.out(), static_cast<CharT>('"'));
                out = write<CharT>(out, static_cast<std::basic_string_view<CharT>>(value));
                return write<CharT>(out, static_cast<CharT>('"'));
            }
            else
                return value_formatter.format(as_leaf(value), context);
        }
    };
}

namespace csl::ag::concepts {
    template <typename T>
    concept produced = requires {
        typename T::csl_ag_product;
    };
}

// TODO(Guss): for_each(_fields)(aggregate auto &&, visitor F&&)
//  [ ] std::hash
//  [ ] comparator
//  [ ] projections
// TODO(Guss): opt-in(s) ?
//  [ ] operator==
//  [ ] operator= / assign

#undef csl_fwd
