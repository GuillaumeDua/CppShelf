#pragma once

#include <csl/workflow.hpp>

namespace test::wf::operators::pipe {
    consteval void overload_rvalue_references() {

        using namespace csl::wf::operators;
        constexpr auto route = 
            [](int)  constexpr {}  |
            [](char) constexpr {}  |
            [](bool) constexpr {}
        ;

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), bool>);
    }
    consteval void overload_lvalue_references() {
        auto a = [](int){};
        auto b = [](char){};
        auto c = [](bool){};

        using namespace csl::wf::operators;
        constexpr auto route = a | b | c;

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), bool>);
    }
    consteval void overload_const_lvalue_references() {
        const auto a = [](int){};
        const auto b = [](char){};
        const auto c = [](bool){};

        using namespace csl::wf::operators;
        constexpr auto route = a | b | c;

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), bool>);
    }
    consteval void overload_mix_cvrefs() {
              auto a = [](int){};
        const auto b = [](char){};

        using namespace csl::wf::operators;
        constexpr auto route = a | b | [](bool){};

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), bool>);
    }

    consteval void collapse_overloads() {

        using namespace csl::wf::operators;
        constexpr auto route = 
            csl::wf::details::overload{
                [](int)  constexpr {},
                [](char) constexpr {}
            } |
            csl::wf::details::overload{
                [](double)  constexpr {},
                [](float)   constexpr {}
            } |
            [](bool) constexpr {}
        ;
        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), double>);
        static_assert(std::is_invocable_v<decltype(route), float>);
        static_assert(std::is_invocable_v<decltype(route), bool>);
    }
}