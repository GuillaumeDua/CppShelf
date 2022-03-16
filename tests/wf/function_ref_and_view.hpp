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
}
  
namespace test::wf::function_ref_ {

    using functor_type = test::wf::function_ref_and_view_::details::functor_type;

    consteval void construct_copy_assign() {

        constexpr auto check = []<typename func_ref_t>(){ 
            static_assert(std::is_nothrow_constructible_v<func_ref_t, typename func_ref_t::type>);

            static_assert(not std::is_default_constructible_v<func_ref_t>);
            static_assert(std::is_trivially_move_constructible_v<func_ref_t>);
            static_assert(std::is_nothrow_move_constructible_v<func_ref_t>);
            static_assert(std::is_trivially_copy_constructible_v<func_ref_t>);
            static_assert(std::is_nothrow_copy_constructible_v<func_ref_t>);

            static_assert(std::is_trivially_copy_assignable_v<func_ref_t>);
            static_assert(std::is_nothrow_copy_assignable_v<func_ref_t>);
            static_assert(std::is_trivially_move_assignable_v<func_ref_t>);
            static_assert(std::is_nothrow_move_assignable_v<func_ref_t>);
        };
    
        check.template operator()<csl::wf::function_ref<functor_type>>();
        check.template operator()<csl::wf::function_ref<const functor_type>>();
    }
    consteval void invoke_operator() {
        {   // T &, T &&
            using function_ref_t = csl::wf::function_ref<functor_type>;
            static_assert(std::same_as<
                functor_type &,
                decltype(std::declval<function_ref_t&>().template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                functor_type &&,
                decltype(std::declval<function_ref_t&&>().template operator()<int>('a'))
            >);
        }
        {   // const T &, const T &&
            using function_ref_t = csl::wf::function_ref<const functor_type>;
            static_assert(std::same_as<
                const functor_type &,
                decltype(std::declval<function_ref_t&>().template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                const functor_type &&,
                decltype(std::declval<function_ref_t&&>().template operator()<int>('a'))
            >);
        }
    }
}
namespace test::wf::function_view_ {

    using functor_type = test::wf::function_ref_and_view_::details::functor_type;

    consteval void construct_copy_assign() {

        constexpr auto check = []<typename func_ref_t>(){ 
            static_assert(std::is_nothrow_constructible_v<func_ref_t, typename func_ref_t::type>);

            static_assert(not std::is_default_constructible_v<func_ref_t>);
            static_assert(std::is_trivially_move_constructible_v<func_ref_t>);
            static_assert(std::is_nothrow_move_constructible_v<func_ref_t>);
            static_assert(std::is_copy_constructible_v<func_ref_t>);
            static_assert(std::is_nothrow_copy_constructible_v<func_ref_t>);

            static_assert(not std::is_trivially_copy_assignable_v<func_ref_t>);
            static_assert(not std::is_trivially_move_assignable_v<func_ref_t>);
        };
    
        check.template operator()<csl::wf::function_view<functor_type>>();
        check.template operator()<csl::wf::function_view<const functor_type>>();
    }
    consteval void invoke_operator() {
        {   // T &, T &&
            using function_ref_t = csl::wf::function_view<functor_type>;
            static_assert(std::same_as<
                functor_type &,
                decltype(std::declval<function_ref_t&>().template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                functor_type &&,
                decltype(std::declval<function_ref_t&&>().template operator()<int>('a'))
            >);
        }
        {   // const T &, const T &&
            using function_ref_t = csl::wf::function_view<const functor_type>;
            static_assert(std::same_as<
                const functor_type &,
                decltype(std::declval<function_ref_t&>().template operator()<int>('a'))
            >);
            static_assert(std::same_as<
                const functor_type &&,
                decltype(std::declval<function_ref_t&&>().template operator()<int>('a'))
            >);
        }
    }
}