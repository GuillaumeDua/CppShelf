#pragma once

#include <csl/wf.hpp>

namespace test::wf::operators::pipe {
    // consteval void overload_rvalue_references() {

    //     using namespace csl::wf::operators;
    //     constexpr auto route = 
    //         [](int)  constexpr {}  |
    //         [](char) constexpr {}  |
    //         [](bool) constexpr {}
    //     ;

    //     static_assert(std::is_invocable_v<decltype(route), int>);
    //     static_assert(std::is_invocable_v<decltype(route), char>);
    //     static_assert(std::is_invocable_v<decltype(route), bool>);
    // }
    // consteval void overload_lvalue_references() {
    //     auto a = [](int){};
    //     auto b = [](char){};
    //     auto c = [](bool){};

    //     using namespace csl::wf::operators;
    //     constexpr auto route = a | b | c;

    //     static_assert(std::is_invocable_v<decltype(route), int>);
    //     static_assert(std::is_invocable_v<decltype(route), char>);
    //     static_assert(std::is_invocable_v<decltype(route), bool>);
    // }
    // consteval void overload_const_lvalue_references() {
    //     const auto a = [](int){};
    //     const auto b = [](char){};
    //     const auto c = [](bool){};

    //     using namespace csl::wf::operators;
    //     constexpr auto route = a | b | c;

    //     static_assert(std::is_invocable_v<decltype(route), int>);
    //     static_assert(std::is_invocable_v<decltype(route), char>);
    //     static_assert(std::is_invocable_v<decltype(route), bool>);
    // }
    // consteval void overload_mix_cvrefs() {
    //           auto a = [](int){};
    //     const auto b = [](char){};

    //     using namespace csl::wf::operators;
    //     constexpr auto route = a | b | [](bool){};

    //     static_assert(std::is_invocable_v<decltype(route), int>);
    //     static_assert(std::is_invocable_v<decltype(route), char>);
    //     static_assert(std::is_invocable_v<decltype(route), bool>);
    // }

    consteval void flatten_overloads() {

        using namespace csl::wf::operators;

        auto f1 = [](int)    constexpr {};
        auto f2 = [](char)   constexpr {};
        auto f3 = [](double) constexpr {};
        auto f4 = [](float)  constexpr {};
        auto f5 = [](bool)   constexpr {};

        auto node_piece_1 = csl::wf::details::overload{ f1, f2 };
        auto node_piece_2 = csl::wf::details::overload{ f3, f4 };
        auto node_piece_3 = f5;

        constexpr auto route = 
            node_piece_1
        |   node_piece_2
        |   node_piece_3
        ;

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), double>);
        static_assert(std::is_invocable_v<decltype(route), float>);
        static_assert(std::is_invocable_v<decltype(route), bool>);

        static_assert(std::same_as<
            const csl::wf::details::overload<
                std::remove_cvref_t<decltype(f1)>,
                std::remove_cvref_t<decltype(f2)>,
                std::remove_cvref_t<decltype(f3)>,
                std::remove_cvref_t<decltype(f4)>,
                std::remove_cvref_t<decltype(f5)>
            >,
            decltype(route)
        >);
    }
}