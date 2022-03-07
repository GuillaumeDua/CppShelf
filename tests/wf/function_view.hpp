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


    consteval void constructors_and_assignements() {

        constexpr auto checks = []<typename func_ref_t>(){ 
            static_assert(not std::is_default_constructible_v<func_ref_t>);
            static_assert(not std::is_copy_assignable_v<func_ref_t>);
            static_assert(not std::is_move_assignable_v<func_ref_t>);

            static_assert(std::is_trivially_move_constructible_v<func_ref_t>);
            static_assert(std::is_copy_constructible_v<func_ref_t>);
        };

        checks.template operator()<csl::wf::function_view<functor_type&>>();
        checks.template operator()<csl::wf::function_view<functor_type&&>>();
        checks.template operator()<csl::wf::function_view<const functor_type&>>();
        checks.template operator()<csl::wf::function_view<const functor_type&&>>();
    }

    namespace details {
        template <typename expected_type, typename func_ref_t>
        constexpr void test_invoke_synthaxes() {

            static_assert(std::same_as<
                expected_type,
                decltype(std::declval<func_ref_t>().template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke<int>(std::declval<func_ref_t>(), csl::wf::mp::ttps<int>{}, 'a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke<int>(std::declval<func_ref_t>(), 'a'))
            >);
            static_assert(std::same_as<
                expected_type,
                decltype(csl::wf::invoke(std::declval<func_ref_t>(), csl::wf::mp::ttps<int>{}, 'a'))
            >);
        };
    }

    consteval void cvref_qualifiers() {

        constexpr auto test_function_ref_cvref_qualifier = []<typename expected_type>(){

            using func_ref_t = decltype(csl::wf::function_view{ std::declval<expected_type>() });

            details::test_invoke_synthaxes<expected_type, func_ref_t&>();
            details::test_invoke_synthaxes<expected_type, func_ref_t&&>();
            details::test_invoke_synthaxes<expected_type, const func_ref_t&>();
            details::test_invoke_synthaxes<expected_type, const func_ref_t &&>();
        };

        test_function_ref_cvref_qualifier.template operator()<functor_type&>();
        test_function_ref_cvref_qualifier.template operator()<functor_type&&>();
        test_function_ref_cvref_qualifier.template operator()<const functor_type&>();
        test_function_ref_cvref_qualifier.template operator()<const functor_type&&>();
    }
}