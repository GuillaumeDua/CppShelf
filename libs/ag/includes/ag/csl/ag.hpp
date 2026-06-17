#pragma once
// cpp shelf library : aggregates utility
// under MIT License - Copyright (c) 2021 Guillaume Dua "Guss"
// https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

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
        constexpr inline static auto value = trait<bound_Ts..., Ts...>::value;
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
//  For user-defined aggregate types outside namespace csl::ag, ADL does not search
//  csl::ag either - making get<I> irrecoverably unfindable for them.
//
//  The generated code exists because C++ has no introspection for aggregate field types:
//  make_to_tuple<N> uses structured bindings at consteval time to capture field types
//  as a std::tuple via decltype, producing the std::type_identity<std::tuple<Ts...>>
//  that drives csl::ag::element<I, T> and csl::ag::to_tuple_t<T>.

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
        return std::get<index>(csl_fwd(value));
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

// ---------------------
//  formatting/printing
// ---------------------

#if defined(CSL_AG__ENABLE_IOSTREAM_SUPPORT) or defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) or defined(CSL_AG__ENABLE_STD_FORMAT_SUPPORT)
# define CSL_AG__FORMATTING_ENABLED
# endif

// type_name
#if defined(CSL_AG__FORMATTING_ENABLED)
#pragma region type_name

#if __has_include(<csl/typeinfo.hpp>)
#   include <csl/typeinfo.hpp>
namespace csl::ag::io::details {

    template <typename T>
    struct type_name : csl::typeinfo::type_name<T>{};
    template <typename T>
    constexpr inline static std::string_view type_name_v = type_name<T>::value;
}
#else
#   pragma message("[csl::ag](EXPERIMENTALE) formatting enabled, but <csl/typeinfo.hpp> not available, fallback to <typeindex>. Relies on runtime implementation.")
#   include <typeindex>
namespace csl::ag::io::details {

    template <typename T>
    struct type_name {
        static inline const std::string_view value = std::type_index(typeid(T)).name();
    };
    template <typename T>
    const inline static std::string_view type_name_v = type_name<T>::value;
}
#endif

#pragma endregion
#endif

// Formatting(shared): composable format options, tag types, decorated view, operator|
#if defined(CSL_AG__FORMATTING_ENABLED)

#pragma message("[csl::ag] Formatting enabled")

namespace csl::ag::io {
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

namespace csl::ag::io::details::decorators {

    /// \brief Carries formatting options as compile-time template parameters.
    /// WARNING: Options and Depth as template argument results in poor performance, code-generation bloat.
    template <typename T, format_options Options = format_options::none, std::size_t Depth = 0>
    requires std::same_as<T, std::remove_cvref_t<T>>
    struct formatted_view_t {

        using csl_ag_io_decorator = void;
        using value_type = T;

        /*explicit*/ operator const value_type &() const { return value; } // NOLINT(*-explicit-constructor)
        const value_type & value; // NOLINT(*-non-private-member-variables-in-classes, *-avoid-const-or-ref-data-members)
        constexpr static format_options options = Options;
        constexpr static std::size_t    depth   = Depth;
    };
}

namespace csl::ag::io::details::concepts {
    template <typename T>
    concept decorator = requires { T::csl_ag_io_decorator; };
}

namespace csl::ag::io {

    /// \brief structured_bindable T | option => formatted_view_t
    /// NOTE: excludes format_option tags themselves, which are (incidentally) empty aggregates
    ///       `indented | typenamed` combine tags into a format_options bitmask -> not create a view over a tag.
    template <csl::ag::concepts::structured_bindable T, concepts::format_option Option>
    requires (not concepts::format_option<T>)
    [[nodiscard]] auto operator|(T const & value, Option)
    -> details::decorators::formatted_view_t<std::remove_cvref_t<T>, Option::value>
    {
        return { .value = value };
    }

    /// \brief formatted_view_t | additional option => accumulated view (same depth, format_option)
    template <typename T, format_options Options, std::size_t Depth, concepts::format_option Option>
    [[nodiscard]] auto operator|(details::decorators::formatted_view_t<T, Options, Depth> view, Option)
    -> details::decorators::formatted_view_t<T, Options | Option::value, Depth>
    {
        return { .value = view.value };
    }
}

namespace csl::ag::io::details {

    template <typename T>
    [[nodiscard]] constexpr auto opening_bracket() noexcept -> std::string_view {
        if constexpr (csl::ag::concepts::range_like<T>)         return "[";
        else if constexpr (csl::ag::concepts::tuple_like<T>)    return "(";
        else                                                    return "{";
    }
    template <typename T>
    [[nodiscard]] constexpr auto closing_bracket() noexcept -> std::string_view {
        if constexpr (csl::ag::concepts::range_like<T>)         return "]";
        else if constexpr (csl::ag::concepts::tuple_like<T>)    return ")";
        else                                                    return "}";
    }
}

#endif

#if defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) or defined(CSL_AG__ENABLE_STD_FORMAT_SUPPORT)

namespace csl::ag::io::details {

    template <std::size_t N>
    [[nodiscard]] consteval auto to_chars() noexcept {
        constexpr auto digits = [] {
            std::size_t n = N, count = 1;
            while (n >= 10) { n /= 10; ++count; }
            return count;
        }();
        std::array<char, digits> result{};
        auto idx = N;
        for (std::size_t i = digits; i > 0; --i) { result[i - 1] = '0' + static_cast<char>(idx % 10); idx /= 10; }
        return result;
    }

    template <typename Char, typename OutputIt>
    [[nodiscard]] auto write(OutputIt out, std::basic_string_view<Char> sv) noexcept -> OutputIt {
        for (Char c : sv)
            *out++ = c;
        return out;
    }

    template <typename Char, typename OutputIt>
    [[nodiscard]] auto write(OutputIt out, Char c) noexcept -> OutputIt
    requires requires { *out = c; }
    {
        *out = c;
        return ++out;
    }

    static constexpr std::size_t indentation_width = 4;

    /// \brief brief description Shared logic for both fmt and std::format (parse, format)
    // FormatterTemplate: fmt::formatter or std::formatter
    template <
        template <typename, typename> class FormatterTemplate,
        csl::ag::concepts::structured_bindable T,
        format_options Options,
        std::size_t Depth,
        typename Char
    >
    requires (not details::concepts::decorator<T>)
    class ag_formatter_base {

        static constexpr format_options child_options = Options & ~format_options::no_braces;

        // Deduce children formatters
        template <std::size_t ... indexes>
        static auto make_formatters_tuple(std::index_sequence<indexes...>)
        -> std::tuple<
            FormatterTemplate<
                decorators::formatted_view_t<csl::ag::tuplelike::element_t<indexes, T>, child_options, Depth + 1>,
                Char
            >...
        >;
        using formatters_t = decltype(make_formatters_tuple(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{}));

        formatters_t formatters_{};

        // NOTE: bracket/separator can be cleared by :n at runtime
        std::basic_string_view<Char> opening_bracket_{
            bool(Options & format_options::no_braces)
                ? std::basic_string_view<Char>{}
                : std::basic_string_view<Char>{ opening_bracket<T>() }
        };
        std::basic_string_view<Char> closing_bracket_{
            bool(Options & format_options::no_braces)
            ? std::basic_string_view<Char>{}
            : std::basic_string_view<Char>{ closing_bracket<T>() }
        };

        std::basic_string_view<Char> separator_{
            bool(Options & format_options::no_braces)
            ? std::basic_string_view<Char>{}
            : bool(Options & format_options::indented)
                ? std::basic_string_view<Char>{","}
                : std::basic_string_view<Char>{", "}
        };

        template <std::size_t FieldIndex, typename Context>
        void format_element(const T & value, Context & ctx) const {
            using field_type = csl::ag::tuplelike::element_t<FieldIndex, T>;

            if constexpr (FieldIndex > 0) {
                ctx.advance_to(write<Char>(ctx.out(), separator_));
                if constexpr (bool(Options & format_options::indented))
                    ctx.advance_to(write<Char>(ctx.out(), '\n'));
            }

            // NOTE: Indentation is written by the parent formatter, so child formatter DOES NOT.
            if constexpr (bool(Options & format_options::indented))
                ctx.advance_to(std::fill_n(ctx.out(), (Depth + 1) * indentation_width, Char{' '}));

            if constexpr (bool(Options & format_options::indexed)) {
                ctx.advance_to(write<Char>(ctx.out(), '['));
                constexpr auto index_chars = to_chars<FieldIndex>();
                ctx.advance_to(write<Char>(ctx.out(), std::basic_string_view<Char>{index_chars.data(), index_chars.size()}));
                ctx.advance_to(write<Char>(ctx.out(), ']'));
                ctx.advance_to(write<Char>(ctx.out(), ' '));
            }

            if constexpr (bool(Options & format_options::typenamed)) {
                ctx.advance_to(write<Char>(ctx.out(), std::basic_string_view<Char>{type_name_v<field_type>}));
                ctx.advance_to(write<Char>(ctx.out(), ':'));
                ctx.advance_to(write<Char>(ctx.out(), ' '));
            }

            ctx.advance_to(std::get<FieldIndex>(formatters_).format(csl::ag::tuplelike::get<FieldIndex>(value), ctx));
        }

    public:
        using csl_ag_product = void;

        constexpr auto parse(auto & ctx) {

            // :n spec (runtime override for no_braces).
            auto it  = ctx.begin();
            auto end = ctx.end();
            if (it != end and static_cast<char>(*it) == 'n') {
                ++it;
                opening_bracket_ = {};
                closing_bracket_ = {};
                separator_       = {};
            }
            if (it != end and *it != static_cast<Char>('}'))
                ++it; // skip ':' or other trailing char
            ctx.advance_to(it);

            // NOTE: Propagate parse to child formatters with an empty spec
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
                ([&] {
                    auto empty_ctx = std::remove_cvref_t<decltype(ctx)>(std::basic_string_view<Char>{});
                    std::get<indexes>(formatters_).parse(empty_ctx);
                }(), ...);
            }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});

            return it;
        }

        template <typename Context>
        auto format(const T & value, Context & ctx) const {
            ctx.advance_to(write<Char>(ctx.out(), opening_bracket_));
            if constexpr (bool(Options & format_options::indented))
                ctx.advance_to(write<Char>(ctx.out(), '\n'));

            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
                (format_element<indexes>(value, ctx), ...);
            }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});

            if constexpr (bool(Options & format_options::indented)) {
                ctx.advance_to(write<Char>(ctx.out(), '\n'));
                ctx.advance_to(std::fill_n(ctx.out(), Depth * 4, Char{' '}));
            }
            ctx.advance_to(write<Char>(ctx.out(), closing_bracket_));
            return ctx.out();
        }
    };

    /// \brief Support non-structured-bindable field types.
    /// Only formats the value: the parent already wrote indentation + format-options prefixes (indexed, typenamed).
    template <
        template <typename, typename> class FormatterTemplate,
        typename T,
        format_options Options,
        std::size_t Depth,
        typename Char
    >
    requires (not csl::ag::concepts::structured_bindable<T>)
    class ag_formatter_base_leaf {
        FormatterTemplate<T, Char> value_formatter_{};
    public:
        using csl_ag_product = void;

        constexpr auto parse(auto & ctx) {
            auto empty_ctx = std::remove_cvref_t<decltype(ctx)>(std::basic_string_view<Char>{});
            value_formatter_.parse(empty_ctx);
            return ctx.begin();
        }
        /// \brief Format, quoted
        /// NOTE: `formatter.set_debug_format()` is implemented on GCC-13's libstdc++, or might depend on __cpp_lib_format_ranges
        template <typename Context>
        auto format(const T & value, Context & ctx) const {

            if constexpr (requires { value_formatter_.set_debug_format(); }){
                value_formatter_.set_debug_format();
                return value_formatter_.format(value, ctx);
            }
            else if constexpr (std::same_as<T, Char>) {
                auto out = write<Char>(ctx.out(), static_cast<Char>('\''));
                out = write<Char>(out, value);
                return write<Char>(out, static_cast<Char>('\''));
            }
            else if constexpr (std::convertible_to<T, std::basic_string_view<Char>>) {
                auto out = write<Char>(ctx.out(), static_cast<Char>('"'));
                out = write<Char>(out, static_cast<std::basic_string_view<Char>>(value));
                return write<Char>(out, static_cast<Char>('"'));
            }
            else
                return value_formatter_.format(value, ctx);
        }
    };
}

#endif // fmtlib or std::format base

//  Formatting: ostream support
//
// Provides operator<<(std::ostream &, structured_bindable) via csl::ag::io.
//
// WARNING: prefer fmtlib or std::format support over this when available.
//
//   Compile-time cost:
//      Including <ostream> is one of the heaviest standard headers.
//      Prefer fmtlib (CSL_AG__ENABLE_FMTLIB_SUPPORT) or std::format (CSL_AG__ENABLE_STD_FORMAT_SUPPORT) for significantly faster build times.
//
//   Runtime cost:
//      std::ios_base::xalloc() and std::ios_base::iword() are used to store the active options on the stream.
//      Both involve a global mutex and can be a bottleneck in hot paths or multi-threaded code.
//
// Design:
//  - Composable format_options bitmask selected via IO manipulators (one-shot, reset after each print)
//      or via the view-based operator| API (bypasses iword entirely):
//      os << value                                          (default: braced, compact)
//      os << csl::ag::io::no_braces << value                (flat, naked: no outer brackets or separator)
//      os << csl::ag::io::indented  << value                (multiline, depth-indented)
//      os << csl::ag::io::indexed   << value                (braced with [N] field indexes)
//      os << csl::ag::io::typenamed << value                (braced with TypeName: prefixes)
//      os << (value | csl::ag::io::indented | csl::ag::io::indexed)  (view-based, composable)
//  - Options propagate to nested structured_bindable fields (no_braces is outermost-only)
//  - Leaf values consistent with fmtlib: char => 'x', bool => true/false, string => "..."
//
// Usage: using namespace csl::ag::io; std::cout << my_aggregate;

#if defined(CSL_AG__ENABLE_IOSTREAM_SUPPORT) and CSL_AG__ENABLE_IOSTREAM_SUPPORT

#pragma message("[csl::ag] CSL_AG__ENABLE_IOSTREAM_SUPPORT - enabled")

#include <ostream>
#include <sstream>

namespace csl::ag::io::details {

    /// \brief T has an operator<<(std::ostream &, T) reachable WITHOUT csl::ag::io in scope.
    // This prevents this library operator<< from satisfying the concept (depend on self).
    template <typename T>
    concept ostream_formattable = requires(std::ostream & os, const std::remove_cvref_t<T> & v) {
        os << v;
    };

    static inline auto mode_index() noexcept -> int {
        static const int idx = std::ios_base::xalloc();
        return idx;
    }

    constexpr static std::size_t indentation_width = 4;

    inline void write_indent(std::ostream & os, std::size_t depth) {
        constexpr std::size_t max_depth = 32;
        static constexpr auto buf =
            []<std::size_t... Is>(std::index_sequence<Is...>) {
                return std::array<char, sizeof...(Is)>{((void)Is, ' ')...};
            }(std::make_index_sequence<max_depth * indentation_width>{});
        os.write(buf.data(), static_cast<std::streamsize>(std::min(max_depth, depth) * indentation_width));
    }

    struct format_options_view {

        bool is_indented;
        bool is_no_braces;
        bool is_indexed;
        bool is_typenamed;
        format_options nested; // no_braces is outermost-only

        constexpr explicit format_options_view(format_options options) noexcept
            : is_indented  { bool(options & format_options::indented)  }
            , is_no_braces { bool(options & format_options::no_braces) }
            , is_indexed   { bool(options & format_options::indexed)   }
            , is_typenamed { bool(options & format_options::typenamed) }
            , nested       { options & ~format_options::no_braces      }
        {}
    };

    // NOTE: Forward declaration: print and print_field_value are mutually recursive.
    template <csl::ag::concepts::structured_bindable T>
    void print(std::ostream & os, T && value, format_options options, std::size_t depth)
    requires (not std::is_array_v<std::remove_cvref_t<T>>);

    /// \brief print_field_value: write one field value of type T to an ostream value.
    // Quoting is consistent with fmtlib: char => 'x', bool => true/false, string => "...".
    // WARNING(Known limitation) range-like and tuple-like field values are not recursively pretty-printed;
    //   they require a user-provided operator<< or they hit the static_assert.
    // TODO: get rid of such a limitation, for consistency sake ?
    template <typename T>
    void print_field_value(std::ostream & os, T && value, format_options options, std::size_t depth) {
        using type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<type, bool>)
            os << (value ? "true" : "false");
        else if constexpr (std::is_same_v<type, char>)
            os << '\'' << value << '\'';
        else if constexpr (std::is_same_v<type, std::string_view> or std::is_same_v<type, std::string>)
            os << '"' << value << '"';
        else if constexpr (ostream_formattable<type>)
            os << value;
        else if constexpr (csl::ag::concepts::structured_bindable<type> and not std::is_array_v<type>)
            print(os, csl_fwd(value), options, depth);
        else
            static_assert(false, "[csl::ag::io] field type is not printable: provide operator<<(std::ostream &, T)");
    }

    template <csl::ag::concepts::structured_bindable T>
    void print(std::ostream & os, T && value, format_options options, std::size_t depth)
    requires (not std::is_array_v<std::remove_cvref_t<T>>)
    {
        using type = std::remove_cvref_t<T>;
        constexpr auto size = csl::ag::tuplelike::size_v<type>;

        const auto opt = format_options_view{ options };

        if (not opt.is_no_braces) os << opening_bracket<type>();
        if (opt.is_indented)      os << '\n';

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
            ([&] {
                
                if constexpr (indexes > 0) {
                    if (not opt.is_no_braces)       os << ',';
                    if (opt.is_indented)            os << '\n';
                    else if (not opt.is_no_braces)  os << ' ';
                }
                
                if (opt.is_indented)
                    write_indent(os, depth + 1);

                if (opt.is_indexed)
                    os << '[' << indexes << "] ";
                
                if (opt.is_typenamed) {
                    using field_type = csl::ag::tuplelike::element_t<indexes, type>;
                    os << type_name_v<field_type> << ": ";
                }
                print_field_value(
                    os,
                    csl::ag::tuplelike::get<indexes>(csl_fwd(value)),
                    opt.nested,
                    depth + 1
                );
            }(), ...);
        }(std::make_index_sequence<size>{});

        if (opt.is_indented){
            os << '\n';
            write_indent(os, depth);
        }
        if (not opt.is_no_braces)
            os << closing_bracket<type>();
    }
}

namespace csl::ag::io {

    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - indented
    inline auto operator<<(std::ostream & os, indented_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::indented);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - no_braces
    inline auto operator<<(std::ostream & os, no_braces_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::no_braces);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - indexed
    inline auto operator<<(std::ostream & os, indexed_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::indexed);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - typenamed
    inline auto operator<<(std::ostream & os, typenamed_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::typenamed);
        return os;
    }

    /// \brief std::ostream formatting using formatted_view. Effectively bypasses iword, using compile-time options.
    template <typename T, format_options Options, std::size_t Depth>
    static auto operator<<(std::ostream & os, details::decorators::formatted_view_t<T, Options, Depth> const & view)
    -> std::ostream &
    {
        details::print(os, view.value, Options, Depth);
        return os;
    }

    /// \brief Format a structured_bindable into an std::ostream, using iword options (one-shot) and prints.
    /// User-defined operator<<(std::ostream &, T) wins via overload resolution (exact match).
    auto operator<<(std::ostream & os, const csl::ag::concepts::structured_bindable auto & value)
    -> std::ostream &
    requires (not std::is_array_v<std::remove_cvref_t<decltype(value)>>)
    and (not details::concepts::decorator<std::remove_cvref_t<decltype(value)>>)
    {
        auto options = static_cast<format_options>(os.iword(details::mode_index()));
        os.iword(details::mode_index()) = 0; // reset (one-shot semantics)
        details::print(os, value, options, 0);
        return os;
    }

}

#endif // CSL_AG__ENABLE_IOSTREAM_SUPPORT

// Opt-in: fmt support
#if defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) and not __has_include(<fmt/format.h>)
    static_assert(false, "csl::ag: [CSL_AG_ENABLE_FMTLIB_SUPPORT] set to [true], but header <fmt/format.h> is missing. Did you forget a dependency ?");
#elif defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) and CSL_AG__ENABLE_FMTLIB_SUPPORT

#pragma message("[csl::ag] CSL_AG__ENABLE_FMTLIB_SUPPORT - enabled")

# include <fmt/ranges.h>
# include <fmt/compile.h>

namespace csl::ag::io::type_traits {

    // formatter_value_type
    template <typename T>
    struct formatter_value_type;
    template <typename T, typename Char>
    struct formatter_value_type<fmt::formatter<T, Char>> : std::type_identity<T>{};
    template <typename T>
    using formatter_value_type_t = formatter_value_type<T>::type;

}

// string literals
namespace csl::ag::io::details::concepts {
    template <typename T, typename Char>
    concept string_view_of =
        std::same_as<T, std::basic_string_view<Char>>
    or  std::same_as<T, fmt::basic_string_view<Char>>
    ;

}

namespace csl::ag::io::details {

    template <typename Char, Char... C>
    class string_literal {
        static constexpr Char value[sizeof...(C)] = { C... }; // NOLINT(*-c-arrays)
    public:
        template <concepts::string_view_of<Char> T>
        constexpr operator T() const { return {value, sizeof...(C)}; } // NOLINT(*-explicit-constructor)
    };
    template <typename Char>
    struct string_literal<Char> {
        template <concepts::string_view_of<Char> T>
        constexpr operator T() const { return {}; }// NOLINT(*-explicit-constructor)
    };

    template <typename Char, Char ... lhs_C, Char ... rhs_C>
    [[nodiscard]] constexpr auto concat(string_literal<Char, lhs_C...>, string_literal<Char, rhs_C...>){
        return string_literal<Char, lhs_C..., rhs_C...>{};
    }
}

// TODO(Guillaume) per-style struct, default is set using a PP (possibly, cmake-provided variable)
//  Might add an indent value project (like, -1 for instance)
//  ex: clang-format brace-wrapping, AlignAfterOpenBracket, etc. https://clang.llvm.org/docs/ClangFormatStyleOptions.html#bracewrapping
//  For now, we emulates the following style:
//      AlignAfterOpenBracket: BlockIndent # AlwaysBreak
//      BraceWrapping*: Never/false
//      BracedInitializerIndentWidth: 4
//      UseTab: Never
//      IndentWidth: 4
//      TabWidth: 4
namespace csl::ag::io::details::configuration::style {
    // template <typename Char> constexpr inline static fmt::basic_string_view<Char> opening_bracket_v = "{";
    // template <typename Char> constexpr inline static fmt::basic_string_view<Char> closing_bracket_v = "}";
    // template <typename Char> constexpr inline static fmt::basic_string_view<Char> separator_v = ",";

    template <typename Char> constexpr inline static auto opening_bracket_v = string_literal<Char, '{'>{};
    template <typename Char> constexpr inline static auto closing_bracket_v = string_literal<Char, '}'>{};
    template <typename Char> constexpr inline static auto separator_v = string_literal<Char, ','>{};

    namespace indentation {
        template <typename Char>
        constexpr inline static auto char_v = ' ';
        constexpr inline static auto width_v = 4;
    }
}

// aggregate formatter
template <csl::ag::concepts::aggregate T, typename Char>
requires (not fmt::is_range<T, Char>::value)
and fmt::is_tuple_formattable<csl::ag::view_t<const T &>, Char>::value
class fmt::formatter<T, Char> {

    using csl_view_t = csl::ag::view_t<const T &>;
    using formatter_t = fmt::formatter<csl_view_t, Char>;
    formatter_t formatter_;

public:

    using csl_ag_product = void;

    constexpr formatter(){
        namespace style = csl::ag::io::details::configuration::style;
        formatter_.set_brackets(
            style::opening_bracket_v<Char>,
            style::closing_bracket_v<Char>
        );
        formatter_.set_separator(
            csl::ag::io::details::concat(
                style::separator_v<Char>,
                csl::ag::io::details::string_literal<Char, ' '>{})
        );
    }

    constexpr auto parse(fmt::format_parse_context& ctx) {
        // WARNING: spreading the parse_context to underlying aggregates/tuples/ranges depends on FMT_TUPLE_JOIN_SPECIFIERS,
        // which is experimentale
        return formatter_.parse(ctx);
    }
    template <typename FormatContext>
    auto format(const T & value, FormatContext& ctx) const {
        return formatter_.format(csl::ag::to_tuple_view(value), ctx);
    }
};

// fmt_formatter alias: normalises fmt::formatter's 3-param signature to 2 params
// so it can be passed as a template template parameter to ag_formatter_base.
namespace csl::ag::io::details {
    template <typename T, typename Char = char>
    using fmt_formatter = fmt::formatter<T, Char>;
}

// fmt_formattable (used upstream: not formatter detection)
namespace csl::ag::io::details::type_traits {
    template <typename T, typename Char>
    struct is_fmt_formattable : fmt::is_formattable<T, Char>{};
    template <csl::ag::concepts::structured_bindable T, typename Char>
    struct is_fmt_formattable<T, Char> {
        constexpr static auto value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and fmt::is_formattable<csl::ag::tuplelike::element_t<indexes, T>>::value);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});
    };
    template <typename T, typename Char>
    constexpr inline static auto is_fmt_formattable_v = is_fmt_formattable<T, Char>::value;
}
namespace csl::ag::io::details::concepts {

    template <typename T, typename Char>
    concept fmt_formattable = type_traits::is_fmt_formattable_v<T, Char>;
}

#pragma region // formatted_view_t formatters (composable options)

namespace csl::ag::io::type_traits {
    template <typename T, format_options Options, std::size_t Depth, typename Char>
    struct formatter_value_type<
        fmt::formatter<csl::ag::io::details::decorators::formatted_view_t<T, Options, Depth>, Char>
    > : std::type_identity<T>{};
}

// fmt::formatter for formatted_view_t - composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    csl::ag::io::format_options Options,
    std::size_t Depth,
    typename Char
>
requires (not csl::ag::io::details::concepts::decorator<T>)
and csl::ag::io::details::concepts::fmt_formattable<T, Char>
class fmt::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T, Options, Depth>,
    Char
> : public csl::ag::io::details::ag_formatter_base<
        csl::ag::io::details::fmt_formatter, T, Options, Depth, Char
    >
{};

// fmt::formatter for formatted_view_t - non-structured-bindable leaf T
template <
    typename T,
    csl::ag::io::format_options Options,
    std::size_t Depth,
    typename Char
>
requires (not csl::ag::concepts::structured_bindable<T>)
class fmt::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T, Options, Depth>,
    Char
> : public csl::ag::io::details::ag_formatter_base_leaf<
        csl::ag::io::details::fmt_formatter, T, Options, Depth, Char
    >
{};
#pragma endregion

#endif // CSL_AG__ENABLE_FMTLIB_SUPPORT

// Opt-in: std::format support
#if defined(CSL_AG__ENABLE_STD_FORMAT_SUPPORT) and not __has_include(<format>)
    static_assert(false, "csl::ag: [CSL_AG__ENABLE_STD_FORMAT_SUPPORT] set to [true], but header <format> is missing.");
#elif defined(CSL_AG__ENABLE_STD_FORMAT_SUPPORT) and CSL_AG__ENABLE_STD_FORMAT_SUPPORT

#pragma message("[csl::ag] CSL_AG__ENABLE_STD_FORMAT_SUPPORT - enabled")

#include <format>

// std_formatter alias: normalises std::formatter's 2-param signature for use as
// a template template parameter to ag_formatter_base.
namespace csl::ag::io::details {
    template <typename T, typename Char = char>
    using std_formatter = std::formatter<T, Char>;
}

// std::formatter for plain aggregate T - default (braced, flat) output.
// Mirrors the fmtlib fmt::formatter<T> old path for ergonomic use without a view wrapper.
template <csl::ag::concepts::aggregate T, typename Char>
requires (not csl::ag::io::details::concepts::decorator<T>)
and (not std::ranges::range<T>)
struct std::formatter<T, Char>
    : public csl::ag::io::details::ag_formatter_base<
        csl::ag::io::details::std_formatter, T, csl::ag::io::format_options::none, 0, Char
    >
{};

// std::formatter for formatted_view_t - composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    csl::ag::io::format_options Options,
    std::size_t Depth,
    typename Char
>
requires (not csl::ag::io::details::concepts::decorator<T>)
struct std::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T, Options, Depth>,
    Char
> : public csl::ag::io::details::ag_formatter_base<
        csl::ag::io::details::std_formatter, T, Options, Depth, Char
    >
{};

// std::formatter for formatted_view_t - non-structured-bindable leaf T
template <
    typename T,
    csl::ag::io::format_options Options,
    std::size_t Depth,
    typename Char
>
requires (not csl::ag::concepts::structured_bindable<T>)
struct std::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T, Options, Depth>,
    Char
> : public csl::ag::io::details::ag_formatter_base_leaf<
        csl::ag::io::details::std_formatter, T, Options, Depth, Char
    >
{};

#endif // CSL_AG__ENABLE_STD_FORMAT_SUPPORT

// Formatting: to_string
// Backed by whichever formatting support is enabled, favoring std::format, then fmtlib, then iostream.
// Options are selected as a non-type template argument - tags implicitly convert to format_options,
// and compose via operator| (constant-expression, so usable directly as the NTTP).
// Usage: using namespace csl::ag::io;
//      to_string(value)                            (default: braced, compact)
//      to_string<indented>(value)                  (single option)
//      to_string<indented | typenamed>(value)      (composed options)
//      to_string(value | indented | typenamed)     (equivalent, view-based composition)
#if defined(CSL_AG__ENABLE_STD_FORMAT_SUPPORT) and CSL_AG__ENABLE_STD_FORMAT_SUPPORT

#include <string>

namespace csl::ag::io {
    template <format_options Options = format_options::none>
    [[nodiscard]] auto to_string(csl::ag::concepts::structured_bindable auto const & value) -> std::string
    requires (not details::concepts::decorator<std::remove_cvref_t<decltype(value)>>)
    {
        using value_type = std::remove_cvref_t<decltype(value)>;
        return std::format("{}", details::decorators::formatted_view_t<value_type, Options, 0>{ .value = value });
    }
    template <typename T, format_options Options, std::size_t Depth>
    [[nodiscard]] auto to_string(details::decorators::formatted_view_t<T, Options, Depth> const & view) -> std::string {
        return std::format("{}", view);
    }
}

#elif defined(CSL_AG__ENABLE_FMTLIB_SUPPORT) and CSL_AG__ENABLE_FMTLIB_SUPPORT

#include <string>

namespace csl::ag::io {
    template <format_options Options = format_options::none>
    [[nodiscard]] auto to_string(csl::ag::concepts::structured_bindable auto const & value) -> std::string
    requires (not details::concepts::decorator<std::remove_cvref_t<decltype(value)>>)
    {
        using value_type = std::remove_cvref_t<decltype(value)>;
        return fmt::format("{}", details::decorators::formatted_view_t<value_type, Options, 0>{ .value = value });
    }
    template <typename T, format_options Options, std::size_t Depth>
    [[nodiscard]] auto to_string(details::decorators::formatted_view_t<T, Options, Depth> const & view) -> std::string {
        return fmt::format("{}", view);
    }
}

#elif defined(CSL_AG__ENABLE_IOSTREAM_SUPPORT) and CSL_AG__ENABLE_IOSTREAM_SUPPORT

#include <string>

namespace csl::ag::io {
    template <format_options Options = format_options::none>
    [[nodiscard]] auto to_string(csl::ag::concepts::structured_bindable auto const & value) -> std::string
    requires (not details::concepts::decorator<std::remove_cvref_t<decltype(value)>>)
    {
        using value_type = std::remove_cvref_t<decltype(value)>;
        std::ostringstream ss;
        ss << details::decorators::formatted_view_t<value_type, Options, 0>{ .value = value };
        return ss.str();
    }
    template <typename T, format_options Options, std::size_t Depth>
    [[nodiscard]] auto to_string(details::decorators::formatted_view_t<T, Options, Depth> const & view) -> std::string {
        std::ostringstream ss;
        ss << view;
        return ss.str();
    }
}

#endif

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
