#pragma once

#include <csl/wf.hpp>

namespace test::wf::operators::pipe_function_ref_ {

    auto func = []<typename ...>(){}; // NOLINT

    consteval void construct() {
        
        using namespace csl::wf::operators;

        // lvalue-reference
        static_assert(std::same_as<
            decltype(func | ref),
            csl::wf::function_ref<decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(func | ref | ref),
            csl::wf::function_ref<decltype(func)>
        >);
        // rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(func) | ref),
            csl::wf::function_ref<decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(func) | ref | ref),
            csl::wf::function_ref<decltype(func)>
        >);
        // const lvalue-reference
        static_assert(std::same_as<
            decltype(std::as_const(func) | ref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::as_const(func) | ref | ref),
            csl::wf::function_ref<const decltype(func)>
        >);
        // const rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | ref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | ref | ref),
            csl::wf::function_ref<const decltype(func)>
        >);
    }
}