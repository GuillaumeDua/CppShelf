#pragma once

#include <csl/wf.hpp>
#include <memory>
#include <utils/semantic_types.hpp>

namespace test {
    consteval void bind_front_() {
        auto func = []<typename ... Ts>() noexcept { return std::tuple<Ts...>{}; };

        using namespace csl::wf;

        // operator()
        static_assert(std::is_same_v<decltype(bind_front(func)()), std::tuple<>>);
        static_assert(std::is_same_v<decltype(bind_front<int>(func)()), std::tuple<int>>);
        static_assert(std::is_same_v<decltype(bind_front<int, char>(func)()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_front<int>(func).template operator()<char>()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_front(func).template operator()<int, char>()), std::tuple<int, char>>);

        namespace mp = csl::wf::mp;

        // is_invocable
        static_assert(mp::is_invocable_v<decltype(bind_front(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_front<int>(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_front<int, char>(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_front<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_invocable_v<decltype(bind_front(func)), mp::ttps<int, char>>);

        // is_nothrow_invocable
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_front(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_front<int>(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_front<int, char>(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_front<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_front(func)), mp::ttps<int, char>>);

        // is_invocable_r_v
        static_assert(mp::is_invocable_r_v<std::tuple<>,             decltype(bind_front(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int>,          decltype(bind_front<int>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int, char>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front(func)), mp::ttps<int, char>>);

        // is_nothrow_invocable_r_v
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<>,             decltype(bind_front(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int>,          decltype(bind_front<int>(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int, char>(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_front(func)), mp::ttps<int, char>>);

        // same_as<return_type, r>
        static_assert(std::same_as<std::tuple<>,             mp::invoke_result_t<decltype(bind_front(func))>>);
        static_assert(std::same_as<std::tuple<int>,          mp::invoke_result_t<decltype(bind_front<int>(func))>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_front<int, char>(func))>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_front<int>(func)), mp::ttps<char>>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_front(func)), mp::ttps<int, char>>>);
    }
}
namespace test::front_binder_ {

    constexpr auto func = []<typename T, typename U>(int args_0, char args_1) constexpr {
        static_assert(std::same_as<void, T>);
        static_assert(std::same_as<void, U>);
        return args_0 + args_1;
    };
    using F = decltype(func);
    using namespace csl::wf;

    consteval void declare_construct() {
        {
            using type = front_binder<F, mp::ttps<void, void>, int>;
            {
                constexpr auto value = type{ func, mp::ttps<void, void>{}, 42}; // (1)
                static_assert(value('A') == 107);
            }
            // [[maybe_unused]] auto value = type{ func, mp::ttps<>{}, 42}; // no
            {
                constexpr auto value = type{ func, 42}; // (2)
                static_assert(value('A') == 107);
            }
        }
    }
    consteval void deduce() {
        constexpr auto value = front_binder{ func, mp::ttps<void, void>{}, 42 };
        using expected_type = front_binder<std::decay_t<F>, mp::ttps<void, void>, int>;
        static_assert(std::same_as<
            decltype(value),
            const expected_type
        >);
    }
    consteval void copy() {
        {   // trivial, nothrow
            constexpr auto value = front_binder{ func, mp::ttps<void, void>{}, 42 };

            static_assert(std::is_copy_constructible_v<decltype(value)>);
            static_assert(std::is_trivially_copy_constructible_v<decltype(value)>);
            static_assert(std::is_nothrow_copy_constructible_v<decltype(value)>);

            constexpr auto copy_value = value;
            using expected_type = front_binder<std::decay_t<F>, mp::ttps<void, void>, int>;
            static_assert(std::same_as<
                decltype(copy_value),
                const expected_type
            >);
        }
        {   // can throw, non-trivial copy
            struct copy_and_move_can_throw {
                constexpr copy_and_move_can_throw() = default;
                constexpr copy_and_move_can_throw(const copy_and_move_can_throw&) {}; // NOLINT
                constexpr copy_and_move_can_throw(copy_and_move_can_throw&&) {}; // NOLINT
                constexpr copy_and_move_can_throw & operator=(const copy_and_move_can_throw&) = delete;
                constexpr copy_and_move_can_throw & operator=(copy_and_move_can_throw&&) = delete;
                constexpr ~copy_and_move_can_throw() = default;
                void operator()(){}
            };
            constexpr auto value = front_binder{ copy_and_move_can_throw{} };
            using expected_type = front_binder<copy_and_move_can_throw, mp::ttps<>>;
            static_assert(std::same_as<
                decltype(value),
                const expected_type
            >);

            static_assert(std::is_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_trivially_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_nothrow_copy_constructible_v<decltype(value)>);

            constexpr auto copy_value = value;
            static_assert(std::same_as<
                decltype(copy_value),
                decltype(value)
            >);
        }
        {   // not_copiable, non-trivial move
            using not_copyable = tests::details::utils::not_copyable;
            constexpr auto value = front_binder{ not_copyable{} };

            static_assert(not std::is_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_trivially_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_nothrow_copy_constructible_v<decltype(value)>);

            using expected_type = front_binder<not_copyable, mp::ttps<>>;
            static_assert(std::same_as<
                decltype(value),
                const expected_type
            >);
        }
    }
    consteval void move() {
        {   // trivial, nothrow
            auto value = front_binder{ func, mp::ttps<void, void>{}, 42 };

            static_assert(std::is_move_constructible_v<decltype(value)>);
            static_assert(not std::is_trivially_move_constructible_v<decltype(value)>);
            static_assert(std::is_nothrow_move_constructible_v<decltype(value)>);

            auto move_value = std::move(value);
            static_assert(std::same_as<
                decltype(move_value),
                decltype(value)
            >);
        }
        {
            using not_moveable = tests::details::utils::not_moveable;
            auto f = not_moveable{};
            auto value = front_binder{ f, mp::ttps<void, void>{}, f };

            static_assert(std::is_move_constructible_v<decltype(value)>);
            static_assert(std::is_trivially_move_constructible_v<decltype(value)>);
            static_assert(std::is_nothrow_move_constructible_v<decltype(value)>);

            auto move_value = std::move(value);
            static_assert(std::same_as<
                decltype(move_value),
                decltype(value)
            >);
        }
    }
}