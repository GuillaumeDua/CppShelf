#pragma once

#include <csl/wf.hpp>

namespace test::wf::function_ref_and_view_::details {

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

    template <template <typename> typename function_wrapper_t>
    struct test_for {

        consteval static void cvref_qualifiers() {

            constexpr auto test_function_view_cvref_qualifier = []<typename expected_type>(){

                using func_ref_t = decltype(function_wrapper_t{ std::declval<expected_type>() });

                test_invoke_synthaxes<expected_type, func_ref_t&>();
                test_invoke_synthaxes<expected_type, func_ref_t&&>();
                test_invoke_synthaxes<expected_type, const func_ref_t&>();
                test_invoke_synthaxes<expected_type, const func_ref_t &&>();
            };

            test_function_view_cvref_qualifier.template operator()<functor_type&>();
            test_function_view_cvref_qualifier.template operator()<functor_type&&>();
            test_function_view_cvref_qualifier.template operator()<const functor_type&>();
            test_function_view_cvref_qualifier.template operator()<const functor_type&&>();
        }

    private:
        template <typename expected_type, typename func_ref_t>
        constexpr static void test_invoke_synthaxes() {

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
        }
    };

    template <template <typename> typename function_wrapper_t>
    consteval void check_constructors_and_assigments(auto checks) {
        checks.template operator()<function_wrapper_t<details::functor_type&>>();
        checks.template operator()<function_wrapper_t<details::functor_type&&>>();
        checks.template operator()<function_wrapper_t<const details::functor_type&>>();
        checks.template operator()<function_wrapper_t<const details::functor_type&&>>();
    }
}
namespace test::wf::function_ref_and_view_ {

    consteval void constructors_and_assigments() {

        details::check_constructors_and_assigments<csl::wf::function_view>(
            []<typename func_ref_t>(){ 
                static_assert(std::is_nothrow_constructible_v<func_ref_t, typename func_ref_t::value_type>);

                static_assert(not std::is_default_constructible_v<func_ref_t>);
                static_assert(std::is_trivially_move_constructible_v<func_ref_t>);
                static_assert(std::is_nothrow_move_constructible_v<func_ref_t>);
                static_assert(std::is_copy_constructible_v<func_ref_t>);
                static_assert(std::is_nothrow_copy_constructible_v<func_ref_t>);

                static_assert(not std::is_trivially_copy_assignable_v<func_ref_t>);
                static_assert(not std::is_trivially_move_assignable_v<func_ref_t>);
            }
        );
        details::check_constructors_and_assigments<csl::wf::function_ref>(
            []<typename func_ref_t>(){ 
                static_assert(std::is_nothrow_constructible_v<func_ref_t, typename func_ref_t::value_type>);

                static_assert(not std::is_default_constructible_v<func_ref_t>);
                static_assert(std::is_trivially_move_constructible_v<func_ref_t>);
                static_assert(std::is_nothrow_move_constructible_v<func_ref_t>);
                static_assert(std::is_trivially_copy_constructible_v<func_ref_t>);
                static_assert(std::is_nothrow_copy_constructible_v<func_ref_t>);

                static_assert(std::is_trivially_copy_assignable_v<func_ref_t>);
                static_assert(std::is_nothrow_copy_assignable_v<func_ref_t>);
                static_assert(std::is_trivially_move_assignable_v<func_ref_t>);
                static_assert(std::is_nothrow_move_assignable_v<func_ref_t>);
            }
        );
    }

    consteval void cvref_qualifiers() {
        details::test_for<csl::wf::function_ref>::cvref_qualifiers();
        details::test_for<csl::wf::function_view>::cvref_qualifiers();
    }
}