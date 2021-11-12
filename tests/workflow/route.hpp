#pragma once

#include "../../workflow/workflow.hpp"
#include <bitset>

namespace test::route_ {
    auto node_1 = []()      { return 41; };
    auto node_2 = [](int i) { return i + 1; };
    auto node_3 = []()      { };

#pragma region fwd (node_1 >[int]> node_2)
    // std::invocable
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1 })
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2 })
    >);

    // invoke_result_t
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1 })>,
        int
    >);
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1, node_2 })>,
        int
    >);
#pragma endregion
#pragma region fwd (node_2 >[int]> node_2 >[int]> node_2) 
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2 }),
        int
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2, node_2 }),
        int
    >);
#pragma endregion

#pragma region discard (node_1 >[int]> node_3)
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_3 })
    >);
#pragma endregion
#pragma region fwd, discard (node_1 >[int]> node_2 >> node_3)
   
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2, node_3 })
    >);

#pragma endregion

    // todo : use deleted operator() instead

    template <typename T, typename U>
    struct is_same_cvref_qualifiers {
        constexpr static bool value = (
                std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U>
            and std::is_rvalue_reference_v<T> == std::is_rvalue_reference_v<U>
            and std::is_const_v<std::remove_reference_t<T>> == std::is_const_v<std::remove_reference_t<U>>
            and std::is_volatile_v<std::remove_reference_t<T>> == std::is_volatile_v<std::remove_reference_t<U>>
        );
    };
    template <typename T, typename U>
    constexpr bool is_same_cvref_qualifiers_v = is_same_cvref_qualifiers<T, U>::value;

    template <typename expected>
    struct cvref_validator_functor {
        using type = int; // doesnt matter
        void operator()() & {
            static_assert(is_same_cvref_qualifiers_v<expected, type&>);
        }
        void operator()() && {
            static_assert(is_same_cvref_qualifiers_v<expected, type&&>);
        }
        void operator()() const & {
            static_assert(is_same_cvref_qualifiers_v<expected, const type&>);
        }
        void operator()() const && {
            static_assert(is_same_cvref_qualifiers_v<expected, const type &&>);
        }
    };

    consteval void call_cvref_correctness() {

        using lvalue_node_type          = cvref_validator_functor<int&>;
        using const_lvalue_node_type    = cvref_validator_functor<const int&>;
        using rvalue_node_type          = cvref_validator_functor<int&&>;
        using const_rvalue_node_type    = cvref_validator_functor<const int&&>;

        {   // lvalue route
            auto route = csl::wf::route {
                lvalue_node_type{},
                lvalue_node_type{},
                lvalue_node_type{}
            };
            route();
        }
        {   // rvalue route
            csl::wf::route {
                rvalue_node_type{},
                rvalue_node_type{},
                rvalue_node_type{}
            }();
        }
        {   // const lvalue route
            const auto route = csl::wf::route {
                const_lvalue_node_type{},
                const_lvalue_node_type{},
                const_lvalue_node_type{}
            };
            route();
        }
        {   // const rvalue route
            const auto route = csl::wf::route {
                const_rvalue_node_type{},
                const_rvalue_node_type{},
                const_rvalue_node_type{}
            };
            static_cast<const decltype(route)&&>(route)();
            //std::move(route)();
        }
    }
}