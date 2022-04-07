#pragma once

namespace tests::details::utils {
    struct not_copyable {
        constexpr not_copyable() = default;
        constexpr not_copyable(const not_copyable&) = delete;
        constexpr not_copyable(not_copyable&&) = default;
        constexpr not_copyable & operator=(const not_copyable&) = delete;
        constexpr not_copyable & operator=(not_copyable&&) = delete;
        constexpr ~not_copyable() = default;
        void operator()(){}
    };
    struct not_moveable {
        constexpr not_moveable() = default;
        constexpr not_moveable(const not_moveable&) = default;
        constexpr not_moveable(not_moveable&&) noexcept = delete;
        constexpr not_moveable & operator=(const not_moveable&) = default;
        constexpr not_moveable & operator=(not_moveable&&) noexcept = delete;
        constexpr ~not_moveable() = default;
        void operator()(){}
    };
}