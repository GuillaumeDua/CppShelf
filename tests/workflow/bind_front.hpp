#pragma once

#include "../../workflow/workflow.hpp"

namespace test::functional {
    consteval void front_binding() {
        auto func = []<typename ... Ts>() noexcept { return std::tuple<Ts...>{}; };

        using namespace workflow::functional;

        static_assert(std::is_same_v<decltype(bind_front(func)()), std::tuple<>>);
        static_assert(std::is_same_v<decltype(bind_front<int>(func)()), std::tuple<int>>);
        static_assert(std::is_same_v<decltype(bind_front<int, char>(func)()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_front<int>(func).template operator()<char>()), std::tuple<int, char>>);
        static_assert(std::is_same_v<decltype(bind_front(func).template operator()<int, char>()), std::tuple<int, char>>);

        namespace mp = workflow::functional::mp;

        static_assert(mp::is_invocable_r_v<std::tuple<>,             decltype(bind_front(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int>,          decltype(bind_front<int>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int, char>(func))>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front<int>(func)), mp::ttps<char>>);
        static_assert(mp::is_invocable_r_v<std::tuple<int, char>,    decltype(bind_front(func)), mp::ttps<int, char>>);
    }
}