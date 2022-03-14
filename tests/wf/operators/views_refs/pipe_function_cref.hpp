#pragma once

#include <csl/wf.hpp>

// TODO : | ref  | cref
// TODO : | cref | ref

namespace test::wf::operators::pipe_function_cref_ {

    auto func = []<typename ...>(){}; // NOLINT

    consteval void construct() {
        
        using namespace csl::wf::operators;

        // lvalue-reference
        static_assert(std::same_as<
            decltype(func | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(func | cref | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        // rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(func) | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(func) | cref | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        // const lvalue-reference
        static_assert(std::same_as<
            decltype(std::as_const(func) | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::as_const(func) | cref | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        // const rvalue-reference
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | cref),
            csl::wf::function_ref<const decltype(func)>
        >);
        static_assert(std::same_as<
            decltype(std::move(std::as_const(func)) | cref | cref),
            csl::wf::function_ref<const decltype(func)>
        >);

    }
}