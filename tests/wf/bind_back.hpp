#pragma once

#include <csl/wf.hpp>
#include <memory>
#include <utils/semantic_types.hpp>

namespace test::back_binder_ {

    constexpr auto func = []<typename T, typename U>(int args_0, char args_1) constexpr {
        static_assert(std::same_as<void, T>);
        static_assert(std::same_as<void, U>);
        return args_0 + args_1;
    };
    using F = decltype(func);
    using namespace csl::wf;

    consteval void declare_construct() {
        {
            using type = back_binder<F, mp::ttps<void, void>, mp::args<int>>;
            {
                constexpr auto value = type{ func, mp::ttps<void, void>{}, 42 }; // (1)
                static_assert(value('A') == 107);
            }
            // [[maybe_unused]] auto value = type{ func, mp::ttps<>{}, 42}; // no
            {
                constexpr auto value = type{ func, 42}; // (2)
                static_assert(value('A') == 107);
            }
        }
    }

    struct user_defined_functor {
        constexpr int operator()(int args_0, char args_1) const noexcept {
            return args_0 + args_1 + storage;
        };
        int storage = 0;
    };

    consteval void underlying_function() {

        using type = back_binder<user_defined_functor, mp::ttps<>, mp::args<int>>;

        constexpr auto value = type{ user_defined_functor{1}, 2 };
        static_assert(value.underlying_function()(2, 39) == 42);
    }
    consteval void noexcept_construct() {
        struct F_noexcept {};
        {
            constexpr auto value = back_binder{ F_noexcept{} };
            static_assert(std::is_nothrow_constructible_v<decltype(value), F_noexcept&&>);
        }
        struct F_can_throw { // NOLINT
            constexpr F_can_throw() = default;
            constexpr F_can_throw(F_can_throw&&) noexcept(false) {}

            constexpr auto operator==(const F_can_throw & other) const noexcept -> bool = default; // fix GCC issue : tuple(Ts &&...) involves get<I>(lhs) == get<I>(rhs) for some reasons ...
        };
        {
            constexpr auto value = back_binder{ F_can_throw{} };
            static_assert(not std::is_nothrow_constructible_v<decltype(value), F_can_throw&&>);
        }
        {
            constexpr auto value = back_binder{ F_noexcept{}, F_can_throw{} };
            static_assert(not std::is_nothrow_constructible_v<decltype(value), F_noexcept&&, F_can_throw&&>);
        }
    }

    consteval void deduce() {
        constexpr auto value = back_binder{ func, mp::ttps<void, void>{}, 42 };
        using expected_type = back_binder<std::decay_t<F>, mp::ttps<void, void>, mp::args<int>>;
        static_assert(std::same_as<
            decltype(value),
            const expected_type
        >);
    }
    
    consteval void copy() {
        {   // trivial, nothrow
            constexpr auto value = back_binder{ func, mp::ttps<void, void>{}, 42 };

            static_assert(std::is_copy_constructible_v<decltype(value)>);
            static_assert(std::is_trivially_copy_constructible_v<decltype(value)>);
            static_assert(std::is_nothrow_copy_constructible_v<decltype(value)>);

            constexpr auto copy_value = value;
            using expected_type = back_binder<std::decay_t<F>, mp::ttps<void, void>, mp::args<int>>;
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
            constexpr auto value = back_binder{ copy_and_move_can_throw{} };
            using expected_type = back_binder<copy_and_move_can_throw, mp::ttps<>, mp::args<>>;
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
            constexpr auto value = back_binder{ not_copyable{} };

            static_assert(not std::is_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_trivially_copy_constructible_v<decltype(value)>);
            static_assert(not std::is_nothrow_copy_constructible_v<decltype(value)>);

            using expected_type = back_binder<not_copyable, mp::ttps<>, mp::args<>>;
            static_assert(std::same_as<
                decltype(value),
                const expected_type
            >);
        }
    }
    consteval void move() {
        {   // trivial, nothrow
            auto value = back_binder{ func, mp::ttps<void, void>{}, 42 }; // NOLINT

            static_assert(std::is_move_constructible_v<decltype(value)>);
            #ifdef __GLIBCXX__
                static_assert(not std::is_trivially_move_constructible_v<decltype(value)>);
            #else
                static_assert(std::is_trivially_move_constructible_v<decltype(value)>);
            #endif
            static_assert(std::is_nothrow_move_constructible_v<decltype(value)>);

            auto move_value = std::move(value);
            static_assert(std::same_as<
                decltype(move_value),
                decltype(value)
            >);
        }
        {   // not_moveable
            using not_moveable = tests::details::utils::not_moveable;
            auto f = not_moveable{};
            auto value = back_binder{ f, mp::ttps<void, void>{}, f };

            // implicit copies
            static_assert(std::is_move_constructible_v<decltype(value)>);
            #if defined(__clang__)
            static_assert(std::is_trivially_move_constructible_v<decltype(value)>);
            #endif
            static_assert(std::is_nothrow_move_constructible_v<decltype(value)>);

            auto move_value = std::move(value);
            static_assert(std::same_as<
                decltype(move_value),
                decltype(value)
            >);
        }
    }
    consteval void swap() {
        using not_moveable = tests::details::utils::not_moveable;
        auto f = not_moveable{};
        auto value = back_binder{ f, mp::ttps<void, void>{}, f };

        static_assert(std::is_nothrow_swappable_v<decltype(value)>);
    }

    consteval void assign_copy() {
        {   // copyable
            auto value = back_binder{ func, mp::ttps<void, void>{}, 42 };

            static_assert(std::is_copy_assignable_v<decltype(value)>);
            static_assert(not std::is_trivially_copy_assignable_v<decltype(value)>);
            static_assert(std::is_nothrow_copy_assignable_v<decltype(value)>);

            std::remove_cvref_t<decltype(value)> other = value;
            other = value;
        }
        {   // not copyable
            using not_copyable = tests::details::utils::not_copyable;
            auto value = back_binder{ not_copyable{}, mp::ttps<>{} };

            static_assert(not std::is_copy_assignable_v<decltype(value)>);
        }
    }
    consteval void assign_move() {
        {   // moveable
            auto value = back_binder{ func, mp::ttps<void, void>{}, 42 };

            static_assert(std::is_move_assignable_v<decltype(value)>);
            static_assert(not std::is_trivially_move_assignable_v<decltype(value)>);
            static_assert(std::is_nothrow_move_assignable_v<decltype(value)>);

            std::remove_cvref_t<decltype(value)> other = value;
            other = std::move(value);
        }
        {   // not moveable
            using not_moveable = tests::details::utils::not_moveable;
            auto f = not_moveable{};
            auto value = back_binder{ f, mp::ttps<>{} };

            static_assert(std::is_move_assignable_v<decltype(value)>); // involve not_moveable copy
        }
    }

    consteval void compare() {
        {   // comparable F
            struct comparable_functor {
                constexpr void operator()(int, int){}
                constexpr auto operator==(const comparable_functor & other) const noexcept -> bool {
                    return std::addressof(other) == this;
                }
            };
            constexpr auto value = back_binder{ comparable_functor{}, 42 };
            static_assert(std::equality_comparable<decltype(value)>);
            static_assert(value == value);
            static_assert(not (value not_eq value));
        }
        {   // not comparable F
            constexpr auto value = back_binder{ func, mp::ttps<void, void>{}, 42 };
            static_assert(not std::equality_comparable<decltype(value)>);
        }
    }
}
namespace test::bind_back_ {
    consteval void type_correctness() {
        auto func = []<typename ... Ts>() noexcept { return std::tuple<Ts...>{}; };

        using namespace csl::wf;

        // operator()
        static_assert(std::is_same_v<decltype(bind_back(func)()), std::tuple<>>);
        static_assert(std::is_same_v<decltype(bind_back<int>(func)()), std::tuple<int>>);
        static_assert(std::is_same_v<decltype(bind_back<int, char>(func)()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_back<char>(func).template operator()<int>()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_back(func).template operator()<int, char>()), std::tuple<int, char>>);

        namespace mp = csl::wf::mp;

        // is_invocable
        static_assert(mp::is_invocable_v<decltype(bind_back(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_back<int>(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_back<int, char>(func))>);
        static_assert(mp::is_invocable_v<decltype(bind_back<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_invocable_v<decltype(bind_back(func)), mp::ttps<int, char>>);

        // is_nothrow_invocable
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_back(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_back<int>(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_back<int, char>(func))>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_back<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_nothrow_invocable_v<decltype(bind_back(func)), mp::ttps<int, char>>);

        // is_invocable_r_v
        static_assert(mp::is_invocable_r_v<std::tuple<>,             decltype(bind_back(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int>,          decltype(bind_back<int>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_back<int, char>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_back<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_back(func)), mp::ttps<int, char>>);

        // is_nothrow_invocable_r_v
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<>,             decltype(bind_back(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int>,          decltype(bind_back<int>(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_back<int, char>(func))>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_back<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_nothrow_invocable_r_v<std::tuple<int, char>,    decltype(bind_back(func)), mp::ttps<int, char>>);

        // same_as<return_type, r>
        static_assert(std::same_as<std::tuple<>,             mp::invoke_result_t<decltype(bind_back(func))>>);
        static_assert(std::same_as<std::tuple<int>,          mp::invoke_result_t<decltype(bind_back<int>(func))>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_back<int, char>(func))>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_back<char>(func)), mp::ttps<int>>>);
        static_assert(std::same_as<std::tuple<int, char>,    mp::invoke_result_t<decltype(bind_back(func)), mp::ttps<int, char>>>);
    }
    consteval void usage_synthaxes() {
        using namespace tests::details::utils;
        using namespace csl::wf;

        auto f = []<typename T, typename U> (strong_of<int> i, strong_of<char> c){
            static_assert(std::same_as<T, void>);
            static_assert(std::same_as<U, int>);
            return i + c;
        };
        auto binder_1 = bind_back<int>(f, strong_of<char>{'A'});
        auto binder_2 = bind_back(f, mp::ttps<int>{}, strong_of<char>{'A'});
        invoke<void>(binder_1, strong_of<int>{-23});
        invoke<void>(binder_2, strong_of<int>{-23});
        
        auto binder_3 = back_binder{ f, mp::ttps<int>{}, strong_of<char>{'A'}};
        binder_3.template operator()<void>(strong_of<int>{-23});

        [[maybe_unused]] auto binder_4 = back_binder<std::remove_cvref_t<decltype(f)>, mp::ttps<int>, mp::args<strong_of<char>>> {
            f, mp::ttps<int>{}, strong_of<char>{'A'}
        };
        [[maybe_unused]] auto binder_5 = back_binder<std::remove_cvref_t<decltype(f)>, mp::ttps<int>, mp::args<strong_of<char>>> {
            f, strong_of<char>{'A'}
        };

        // return
        //     binder_1.template operator()<void>(strong_of<int>{-23}) == 42 and
        //     binder_2.template operator()<void>(strong_of<int>{-23}) == 42
        // ;
    }
}