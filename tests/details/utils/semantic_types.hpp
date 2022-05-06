#pragma once

#include <type_traits>
#include <concepts>

namespace tests::details::utils {
    struct not_copyable {
        constexpr not_copyable() = default;
        constexpr not_copyable(const not_copyable&) = delete;
        constexpr not_copyable(not_copyable&&) = default;
        constexpr not_copyable & operator=(const not_copyable&) = delete;
        constexpr not_copyable & operator=(not_copyable&&) = default;
        constexpr ~not_copyable() = default;

        constexpr auto operator==(const not_copyable & other) const noexcept -> bool = default; // fix GCC issue : tuple(Ts &&...) involves get<I>(lhs) == get<I>(rhs) for some reasons ...

        void operator()(){}
    };
    static_assert(not std::is_copy_constructible_v<not_copyable>);
    static_assert(not std::is_copy_assignable_v<not_copyable>);
    static_assert(std::is_move_constructible_v<not_copyable>);
    static_assert(std::is_move_assignable_v<not_copyable>);
    static_assert(std::equality_comparable<not_copyable>);

    struct not_moveable {
        constexpr not_moveable() = default;
        constexpr not_moveable(const not_moveable&) = default;
        constexpr not_moveable(not_moveable&&) noexcept = delete;
        constexpr not_moveable & operator=(const not_moveable&) = default;
        constexpr not_moveable & operator=(not_moveable&&) noexcept = delete;
        constexpr ~not_moveable() = default;

        constexpr auto operator==(const not_moveable & other) const noexcept -> bool = default; // fix GCC issue : tuple(Ts &&...) involves get<I>(lhs) == get<I>(rhs) for some reasons ...

        void operator()(){}
    };
    static_assert(std::is_copy_constructible_v<not_moveable>);
    static_assert(std::is_copy_assignable_v<not_moveable>);
    static_assert(not std::is_move_constructible_v<not_moveable>);
    static_assert(not std::is_move_assignable_v<not_moveable>);
    static_assert(std::equality_comparable<not_moveable>);

    template <typename T>
    struct strong_of {
        explicit constexpr strong_of(T arg) noexcept : value{arg} {};
        constexpr /* not explicit on purpose*/ operator T() const noexcept { return value; };
    private:
        T value;
    };
}