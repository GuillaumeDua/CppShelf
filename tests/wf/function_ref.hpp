#pragma once

#include <csl/wf.hpp>

namespace test::wf::function_ref_ {

    // operator() [cvref-qualifiers] returns *this with the same cvref-qualifiers
    struct functor_type {
        template <typename T>
        constexpr decltype(auto) operator()(auto && value) & noexcept {
            static_assert(std::same_as<T, int>);
            static_assert(std::same_as<std::remove_cvref_t<decltype(value)>, char>);
            return *this;
        }
        template <typename T>
        constexpr decltype(auto) operator()(auto && value) const & noexcept{
            static_assert(std::same_as<T, int>);
            static_assert(std::same_as<std::remove_cvref_t<decltype(value)>, char>);
            return *this;
        }
        template <typename T>
        constexpr decltype(auto) operator()(auto && value) && noexcept {
            static_assert(std::same_as<T, int>);
            static_assert(std::same_as<std::remove_cvref_t<decltype(value)>, char>);
            return std::move(*this);
        }
        template <typename T>
        constexpr decltype(auto) operator()(auto && value) const && noexcept {
            static_assert(std::same_as<T, int>);
            static_assert(std::same_as<std::remove_cvref_t<decltype(value)>, char>);
            return std::move(*this);
        }
    };

    consteval void cvref_qualifiers() {

        constexpr auto test_invoke_synthaxes = []<typename expected_type>(auto && value) {

            static_assert(std::same_as<
                expected_type,
                decltype(std::forward<decltype(value)>(value).template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke<int>(std::forward<decltype(value)>(value), csl::wf::mp::ttps<int>{}, 'a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke<int>(std::forward<decltype(value)>(value), 'a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke(std::forward<decltype(value)>(value), csl::wf::mp::ttps<int>{}, 'a'))
            >);
        };
        auto test_function_ref_cvref_qualifier = [test_invoke_synthaxes]<typename expected_type>(auto && func){

            auto func_ref = csl::wf::function_ref{ std::forward<decltype(func)>(func) };

            test_invoke_synthaxes.template operator()<expected_type>(func_ref);
            test_invoke_synthaxes.template operator()<expected_type>(std::as_const(func_ref));
            test_invoke_synthaxes.template operator()<expected_type>(std::move(func_ref));
            test_invoke_synthaxes.template operator()<expected_type>(std::move(std::as_const(func_ref)));
        };

        auto func = functor_type{};
        test_function_ref_cvref_qualifier.template operator()<functor_type&>(func);
        test_function_ref_cvref_qualifier.template operator()<functor_type&&>(std::move(func));
        test_function_ref_cvref_qualifier.template operator()<const functor_type&>(std::as_const(func));
        test_function_ref_cvref_qualifier.template operator()<const functor_type&&>(std::move(std::as_const(func)));
    }
}