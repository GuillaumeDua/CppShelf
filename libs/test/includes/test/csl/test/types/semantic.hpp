#pragma once

#include <type_traits>
#if __cplusplus >= 202002L
# include <concepts>
#endif

namespace csl::test::types::semantic {

    struct not_copyable {
        constexpr not_copyable() = default;
        constexpr not_copyable(const not_copyable&) = delete;
        constexpr not_copyable(not_copyable&&) = default;
        constexpr not_copyable & operator=(const not_copyable&) = delete;
        constexpr not_copyable & operator=(not_copyable&&) = default;
        ~not_copyable() = default;

#if __cplusplus >= 202002L
        constexpr auto operator==(const not_copyable & other) const noexcept -> bool = default; // fix GCC issue : tuple(Ts &&...) involves get<I>(lhs) == get<I>(rhs) for some reasons ...
#endif

        void operator()(){}
    };
    static_assert(not std::is_copy_constructible_v<not_copyable>);
    static_assert(not std::is_copy_assignable_v<not_copyable>);
    static_assert(std::is_move_constructible_v<not_copyable>);
    static_assert(std::is_move_assignable_v<not_copyable>);

#if __cplusplus >= 202002L
    static_assert(std::equality_comparable<not_copyable>);
#endif

    struct not_moveable {
        constexpr not_moveable() = default;
        constexpr not_moveable(const not_moveable&) = default;
        constexpr not_moveable(not_moveable&&) noexcept = delete;
        constexpr not_moveable & operator=(const not_moveable&) = default;
        constexpr not_moveable & operator=(not_moveable&&) noexcept = delete;
        ~not_moveable() = default;

#if __cplusplus >= 202002L
        constexpr auto operator==(const not_moveable & other) const noexcept -> bool = default; // fix GCC issue : tuple(Ts &&...) involves get<I>(lhs) == get<I>(rhs) for some reasons ...
#endif

        void operator()(){}
    };
    static_assert(std::is_copy_constructible_v<not_moveable>);
    static_assert(std::is_copy_assignable_v<not_moveable>);
    static_assert(not std::is_move_constructible_v<not_moveable>);
    static_assert(not std::is_move_assignable_v<not_moveable>);
#if __cplusplus >= 202002L
    static_assert(std::equality_comparable<not_moveable>);
#endif

    struct eq_comparable {
        constexpr auto operator==(const eq_comparable &) const noexcept -> bool { return {}; }
    };
    struct not_eq_comparable {
        constexpr auto operator!=(const not_eq_comparable &) const noexcept -> bool { return {}; }
    };
    struct less_than_comparable {
        constexpr auto operator<(const less_than_comparable &) const noexcept -> bool { return {}; }
    };
    struct less_or_eq_comparable {
        constexpr auto operator<=(const less_or_eq_comparable &) const noexcept -> bool { return {}; }
    };
    struct greater_than_comparable {
        constexpr auto operator>(const greater_than_comparable &) const noexcept -> bool { return {}; }
    };
    struct greater_or_eq_comparable {
        constexpr auto operator>=(const greater_or_eq_comparable &) const noexcept -> bool { return {}; }
    };
#if __cplusplus >= 202002L
    static_assert(std::equality_comparable<eq_comparable>);
    static_assert(not std::equality_comparable<not_eq_comparable>);
    static_assert(requires(const less_than_comparable v) { v < v; });
    static_assert(requires(const less_or_eq_comparable v) { v <= v; });
    static_assert(requires(const greater_than_comparable v) { v > v; });
    static_assert(requires(const greater_or_eq_comparable v) { v >= v; });
#endif

    template <typename T>
    struct strong_of {
        explicit constexpr strong_of(T arg) noexcept : value{arg} {};
        constexpr /* not explicit on purpose*/ operator T() const noexcept { return value; }; // NOLINT(*-explicit-*)
    private:
        T value;
    };
}
