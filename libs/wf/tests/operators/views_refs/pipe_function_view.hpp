#pragma once

#include <csl/wf.hpp>

namespace test::wf::operators::pipe_function_view_ {

    auto func = []<typename ...>(){}; // NOLINT

    consteval void construct() {
        
        using namespace csl::wf::operators;

        // lvalue-reference
        static_assert(std::same_as<
            decltype(func | view),
            csl::wf::function_view<decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(func | view | view),
            csl::wf::function_view<decltype(func)>
        >);
        // rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(func) | view),
            csl::wf::function_view<decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(func) | view | view),
            csl::wf::function_view<decltype(func)>
        >);
        // const lvalue-reference
        static_assert(std::same_as<
            decltype(std::as_const(func) | view),
            csl::wf::function_view<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::as_const(func) | view | view),
            csl::wf::function_view<const decltype(func)>
        >);
        // const rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | view),
            csl::wf::function_view<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | view | view),
            csl::wf::function_view<const decltype(func)>
        >);

    }
}