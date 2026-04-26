#pragma once

#include <csl/wf.hpp>

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

    // flattening
    consteval void flatten_overloads_rvalue_ref() {

        using namespace csl::wf::operators;

        auto f1 = [](int)    constexpr {};
        auto f2 = [](char)   constexpr {};
        auto f3 = [](double) constexpr {};
        auto f4 = [](float)  constexpr {};
        auto f5 = [](bool)   constexpr {};

        auto node_piece_1 = csl::wf::details::overload{ std::move(f1), std::move(f2) };
        auto node_piece_2 = csl::wf::details::overload{ std::move(f3), std::move(f4) };
        auto node_piece_3 = std::move(f5);

        constexpr auto route = 
            std::move(node_piece_1)
        |   std::move(node_piece_2)
        |   std::move(node_piece_3)
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
    consteval void flatten_overloads_const_lvalue_ref() {

        using namespace csl::wf::operators;

        const auto f1 = [](int)    constexpr {};
        const auto f2 = [](char)   constexpr {};
        const auto f3 = [](double) constexpr {};
        const auto f4 = [](float)  constexpr {};
        const auto f5 = [](bool)   constexpr {};

        const auto node_piece_1 = csl::wf::details::overload{ f1, f2 };
        const auto node_piece_2 = csl::wf::details::overload{ f3, f4 };
        const auto node_piece_3 = f5;

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