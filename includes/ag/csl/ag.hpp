#pragma once

#include <any>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace csl::ag::details {

    template <typename T>
    constexpr auto declval() noexcept -> T  {
        // static_assert([](){ return false; }(), "csl::ag::details : declval : for unevaluated context only !");
        return *((std::remove_reference_t<T>*){ nullptr });
    }

    template <std::size_t>
    struct field_evaluator {
        explicit constexpr field_evaluator() = delete;

        // not `return std::declval<T>();`, as clang does not like it
        template <typename T>
        consteval operator T&&() const noexcept {
            return declval<T&&>();
        }
        template <typename T>
        consteval operator T&() const noexcept {
            return declval<T&>();
        }
    };
}
namespace csl::ag::concepts {

	template <typename T>
	concept unqualified_aggregate =
        std::is_aggregate_v<T> and
		not std::is_empty_v<T> and
		not std::is_union_v<T> and
		not std::is_polymorphic_v<T> and
		not std::is_reference_v<T>
	;
    template <typename T>
    concept aggregate = unqualified_aggregate<std::remove_cvref_t<T>>
    ;
    template <typename T, typename... args_ts>
    concept aggregate_constructible_from = unqualified_aggregate<T> and requires { T{ std::declval<args_ts>()... }; }
    ;

    template <typename T, std::size_t size>
    concept aggregate_with_n_fields =
        concepts::aggregate<T> and
        []<std::size_t... indexes>(std::index_sequence<indexes...>) {
            return concepts::aggregate_constructible_from<T, details::field_evaluator<indexes>...>;
        }(std::make_index_sequence<size>{})
    ;
}
namespace csl::ag::details {

	template <concepts::aggregate T, std::size_t indice = sizeof(T)>
    requires (indice > 0)
    constexpr std::size_t fields_count = []() {

        if constexpr (concepts::aggregate_with_n_fields<T, indice>)
            return indice;
        else
            return fields_count<T, indice - 1>;
    }();

    template <typename from> requires std::is_lvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::tie(values...);
    }
    template <typename from> requires std::is_rvalue_reference_v<from>
    constexpr auto fwd_tie(auto && ... values) {
        return std::forward_as_tuple(std::move(values)...);
    }

	template <std::size_t N, concepts::aggregate T>
    struct element : std::type_identity<decltype([](){
		static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
	}())>{};

#pragma region as_tuple_impl<N,T>
// GENERATED CONTENT, DO NOT EDIT MANUALLY !
#pragma region CSL_AG_UNFOLD_IDENTITIES_WITH_N

    #define CSL_AG_UNFOLD_IDENTITIES_WITH_1(F) F(v0) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_2(F) F(v1), CSL_AG_UNFOLD_IDENTITIES_WITH_1(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_3(F) F(v2), CSL_AG_UNFOLD_IDENTITIES_WITH_2(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_4(F) F(v3), CSL_AG_UNFOLD_IDENTITIES_WITH_3(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_5(F) F(v4), CSL_AG_UNFOLD_IDENTITIES_WITH_4(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_6(F) F(v5), CSL_AG_UNFOLD_IDENTITIES_WITH_5(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_7(F) F(v6), CSL_AG_UNFOLD_IDENTITIES_WITH_6(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_8(F) F(v7), CSL_AG_UNFOLD_IDENTITIES_WITH_7(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_9(F) F(v8), CSL_AG_UNFOLD_IDENTITIES_WITH_8(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_10(F) F(v9), CSL_AG_UNFOLD_IDENTITIES_WITH_9(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_11(F) F(v10), CSL_AG_UNFOLD_IDENTITIES_WITH_10(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_12(F) F(v11), CSL_AG_UNFOLD_IDENTITIES_WITH_11(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_13(F) F(v12), CSL_AG_UNFOLD_IDENTITIES_WITH_12(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_14(F) F(v13), CSL_AG_UNFOLD_IDENTITIES_WITH_13(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_15(F) F(v14), CSL_AG_UNFOLD_IDENTITIES_WITH_14(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_16(F) F(v15), CSL_AG_UNFOLD_IDENTITIES_WITH_15(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_17(F) F(v16), CSL_AG_UNFOLD_IDENTITIES_WITH_16(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_18(F) F(v17), CSL_AG_UNFOLD_IDENTITIES_WITH_17(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_19(F) F(v18), CSL_AG_UNFOLD_IDENTITIES_WITH_18(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_20(F) F(v19), CSL_AG_UNFOLD_IDENTITIES_WITH_19(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_21(F) F(v20), CSL_AG_UNFOLD_IDENTITIES_WITH_20(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_22(F) F(v21), CSL_AG_UNFOLD_IDENTITIES_WITH_21(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_23(F) F(v22), CSL_AG_UNFOLD_IDENTITIES_WITH_22(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_24(F) F(v23), CSL_AG_UNFOLD_IDENTITIES_WITH_23(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_25(F) F(v24), CSL_AG_UNFOLD_IDENTITIES_WITH_24(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_26(F) F(v25), CSL_AG_UNFOLD_IDENTITIES_WITH_25(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_27(F) F(v26), CSL_AG_UNFOLD_IDENTITIES_WITH_26(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_28(F) F(v27), CSL_AG_UNFOLD_IDENTITIES_WITH_27(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_29(F) F(v28), CSL_AG_UNFOLD_IDENTITIES_WITH_28(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_30(F) F(v29), CSL_AG_UNFOLD_IDENTITIES_WITH_29(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_31(F) F(v30), CSL_AG_UNFOLD_IDENTITIES_WITH_30(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_32(F) F(v31), CSL_AG_UNFOLD_IDENTITIES_WITH_31(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_33(F) F(v32), CSL_AG_UNFOLD_IDENTITIES_WITH_32(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_34(F) F(v33), CSL_AG_UNFOLD_IDENTITIES_WITH_33(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_35(F) F(v34), CSL_AG_UNFOLD_IDENTITIES_WITH_34(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_36(F) F(v35), CSL_AG_UNFOLD_IDENTITIES_WITH_35(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_37(F) F(v36), CSL_AG_UNFOLD_IDENTITIES_WITH_36(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_38(F) F(v37), CSL_AG_UNFOLD_IDENTITIES_WITH_37(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_39(F) F(v38), CSL_AG_UNFOLD_IDENTITIES_WITH_38(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_40(F) F(v39), CSL_AG_UNFOLD_IDENTITIES_WITH_39(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_41(F) F(v40), CSL_AG_UNFOLD_IDENTITIES_WITH_40(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_42(F) F(v41), CSL_AG_UNFOLD_IDENTITIES_WITH_41(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_43(F) F(v42), CSL_AG_UNFOLD_IDENTITIES_WITH_42(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_44(F) F(v43), CSL_AG_UNFOLD_IDENTITIES_WITH_43(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_45(F) F(v44), CSL_AG_UNFOLD_IDENTITIES_WITH_44(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_46(F) F(v45), CSL_AG_UNFOLD_IDENTITIES_WITH_45(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_47(F) F(v46), CSL_AG_UNFOLD_IDENTITIES_WITH_46(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_48(F) F(v47), CSL_AG_UNFOLD_IDENTITIES_WITH_47(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_49(F) F(v48), CSL_AG_UNFOLD_IDENTITIES_WITH_48(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_50(F) F(v49), CSL_AG_UNFOLD_IDENTITIES_WITH_49(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_51(F) F(v50), CSL_AG_UNFOLD_IDENTITIES_WITH_50(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_52(F) F(v51), CSL_AG_UNFOLD_IDENTITIES_WITH_51(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_53(F) F(v52), CSL_AG_UNFOLD_IDENTITIES_WITH_52(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_54(F) F(v53), CSL_AG_UNFOLD_IDENTITIES_WITH_53(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_55(F) F(v54), CSL_AG_UNFOLD_IDENTITIES_WITH_54(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_56(F) F(v55), CSL_AG_UNFOLD_IDENTITIES_WITH_55(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_57(F) F(v56), CSL_AG_UNFOLD_IDENTITIES_WITH_56(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_58(F) F(v57), CSL_AG_UNFOLD_IDENTITIES_WITH_57(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_59(F) F(v58), CSL_AG_UNFOLD_IDENTITIES_WITH_58(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_60(F) F(v59), CSL_AG_UNFOLD_IDENTITIES_WITH_59(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_61(F) F(v60), CSL_AG_UNFOLD_IDENTITIES_WITH_60(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_62(F) F(v61), CSL_AG_UNFOLD_IDENTITIES_WITH_61(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_63(F) F(v62), CSL_AG_UNFOLD_IDENTITIES_WITH_62(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_64(F) F(v63), CSL_AG_UNFOLD_IDENTITIES_WITH_63(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_65(F) F(v64), CSL_AG_UNFOLD_IDENTITIES_WITH_64(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_66(F) F(v65), CSL_AG_UNFOLD_IDENTITIES_WITH_65(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_67(F) F(v66), CSL_AG_UNFOLD_IDENTITIES_WITH_66(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_68(F) F(v67), CSL_AG_UNFOLD_IDENTITIES_WITH_67(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_69(F) F(v68), CSL_AG_UNFOLD_IDENTITIES_WITH_68(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_70(F) F(v69), CSL_AG_UNFOLD_IDENTITIES_WITH_69(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_71(F) F(v70), CSL_AG_UNFOLD_IDENTITIES_WITH_70(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_72(F) F(v71), CSL_AG_UNFOLD_IDENTITIES_WITH_71(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_73(F) F(v72), CSL_AG_UNFOLD_IDENTITIES_WITH_72(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_74(F) F(v73), CSL_AG_UNFOLD_IDENTITIES_WITH_73(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_75(F) F(v74), CSL_AG_UNFOLD_IDENTITIES_WITH_74(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_76(F) F(v75), CSL_AG_UNFOLD_IDENTITIES_WITH_75(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_77(F) F(v76), CSL_AG_UNFOLD_IDENTITIES_WITH_76(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_78(F) F(v77), CSL_AG_UNFOLD_IDENTITIES_WITH_77(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_79(F) F(v78), CSL_AG_UNFOLD_IDENTITIES_WITH_78(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_80(F) F(v79), CSL_AG_UNFOLD_IDENTITIES_WITH_79(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_81(F) F(v80), CSL_AG_UNFOLD_IDENTITIES_WITH_80(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_82(F) F(v81), CSL_AG_UNFOLD_IDENTITIES_WITH_81(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_83(F) F(v82), CSL_AG_UNFOLD_IDENTITIES_WITH_82(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_84(F) F(v83), CSL_AG_UNFOLD_IDENTITIES_WITH_83(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_85(F) F(v84), CSL_AG_UNFOLD_IDENTITIES_WITH_84(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_86(F) F(v85), CSL_AG_UNFOLD_IDENTITIES_WITH_85(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_87(F) F(v86), CSL_AG_UNFOLD_IDENTITIES_WITH_86(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_88(F) F(v87), CSL_AG_UNFOLD_IDENTITIES_WITH_87(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_89(F) F(v88), CSL_AG_UNFOLD_IDENTITIES_WITH_88(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_90(F) F(v89), CSL_AG_UNFOLD_IDENTITIES_WITH_89(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_91(F) F(v90), CSL_AG_UNFOLD_IDENTITIES_WITH_90(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_92(F) F(v91), CSL_AG_UNFOLD_IDENTITIES_WITH_91(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_93(F) F(v92), CSL_AG_UNFOLD_IDENTITIES_WITH_92(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_94(F) F(v93), CSL_AG_UNFOLD_IDENTITIES_WITH_93(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_95(F) F(v94), CSL_AG_UNFOLD_IDENTITIES_WITH_94(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_96(F) F(v95), CSL_AG_UNFOLD_IDENTITIES_WITH_95(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_97(F) F(v96), CSL_AG_UNFOLD_IDENTITIES_WITH_96(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_98(F) F(v97), CSL_AG_UNFOLD_IDENTITIES_WITH_97(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_99(F) F(v98), CSL_AG_UNFOLD_IDENTITIES_WITH_98(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_100(F) F(v99), CSL_AG_UNFOLD_IDENTITIES_WITH_99(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_101(F) F(v100), CSL_AG_UNFOLD_IDENTITIES_WITH_100(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_102(F) F(v101), CSL_AG_UNFOLD_IDENTITIES_WITH_101(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_103(F) F(v102), CSL_AG_UNFOLD_IDENTITIES_WITH_102(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_104(F) F(v103), CSL_AG_UNFOLD_IDENTITIES_WITH_103(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_105(F) F(v104), CSL_AG_UNFOLD_IDENTITIES_WITH_104(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_106(F) F(v105), CSL_AG_UNFOLD_IDENTITIES_WITH_105(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_107(F) F(v106), CSL_AG_UNFOLD_IDENTITIES_WITH_106(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_108(F) F(v107), CSL_AG_UNFOLD_IDENTITIES_WITH_107(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_109(F) F(v108), CSL_AG_UNFOLD_IDENTITIES_WITH_108(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_110(F) F(v109), CSL_AG_UNFOLD_IDENTITIES_WITH_109(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_111(F) F(v110), CSL_AG_UNFOLD_IDENTITIES_WITH_110(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_112(F) F(v111), CSL_AG_UNFOLD_IDENTITIES_WITH_111(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_113(F) F(v112), CSL_AG_UNFOLD_IDENTITIES_WITH_112(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_114(F) F(v113), CSL_AG_UNFOLD_IDENTITIES_WITH_113(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_115(F) F(v114), CSL_AG_UNFOLD_IDENTITIES_WITH_114(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_116(F) F(v115), CSL_AG_UNFOLD_IDENTITIES_WITH_115(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_117(F) F(v116), CSL_AG_UNFOLD_IDENTITIES_WITH_116(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_118(F) F(v117), CSL_AG_UNFOLD_IDENTITIES_WITH_117(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_119(F) F(v118), CSL_AG_UNFOLD_IDENTITIES_WITH_118(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_120(F) F(v119), CSL_AG_UNFOLD_IDENTITIES_WITH_119(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_121(F) F(v120), CSL_AG_UNFOLD_IDENTITIES_WITH_120(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_122(F) F(v121), CSL_AG_UNFOLD_IDENTITIES_WITH_121(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_123(F) F(v122), CSL_AG_UNFOLD_IDENTITIES_WITH_122(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_124(F) F(v123), CSL_AG_UNFOLD_IDENTITIES_WITH_123(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_125(F) F(v124), CSL_AG_UNFOLD_IDENTITIES_WITH_124(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_126(F) F(v125), CSL_AG_UNFOLD_IDENTITIES_WITH_125(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_127(F) F(v126), CSL_AG_UNFOLD_IDENTITIES_WITH_126(F) // NOLINT
#define CSL_AG_UNFOLD_IDENTITIES_WITH_128(F) F(v127), CSL_AG_UNFOLD_IDENTITIES_WITH_127(F) // NOLINT
#pragma endregion
#pragma region as_tuple_impl
    #define IDS(EXPR) EXPR
template <std::size_t N> requires (N == 1) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS));
}
template <std::size_t N> requires (N == 2) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS));
}
template <std::size_t N> requires (N == 3) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS));
}
template <std::size_t N> requires (N == 4) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS));
}
template <std::size_t N> requires (N == 5) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS));
}
template <std::size_t N> requires (N == 6) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS));
}
template <std::size_t N> requires (N == 7) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS));
}
template <std::size_t N> requires (N == 8) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS));
}
template <std::size_t N> requires (N == 9) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS));
}
template <std::size_t N> requires (N == 10) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS));
}
template <std::size_t N> requires (N == 11) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_11(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_11(IDS));
}
template <std::size_t N> requires (N == 12) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_12(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_12(IDS));
}
template <std::size_t N> requires (N == 13) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_13(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_13(IDS));
}
template <std::size_t N> requires (N == 14) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_14(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_14(IDS));
}
template <std::size_t N> requires (N == 15) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_15(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_15(IDS));
}
template <std::size_t N> requires (N == 16) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_16(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_16(IDS));
}
template <std::size_t N> requires (N == 17) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_17(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_17(IDS));
}
template <std::size_t N> requires (N == 18) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_18(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_18(IDS));
}
template <std::size_t N> requires (N == 19) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_19(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_19(IDS));
}
template <std::size_t N> requires (N == 20) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_20(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_20(IDS));
}
template <std::size_t N> requires (N == 21) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_21(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_21(IDS));
}
template <std::size_t N> requires (N == 22) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_22(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_22(IDS));
}
template <std::size_t N> requires (N == 23) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_23(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_23(IDS));
}
template <std::size_t N> requires (N == 24) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_24(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_24(IDS));
}
template <std::size_t N> requires (N == 25) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_25(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_25(IDS));
}
template <std::size_t N> requires (N == 26) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_26(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_26(IDS));
}
template <std::size_t N> requires (N == 27) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_27(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_27(IDS));
}
template <std::size_t N> requires (N == 28) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_28(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_28(IDS));
}
template <std::size_t N> requires (N == 29) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_29(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_29(IDS));
}
template <std::size_t N> requires (N == 30) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_30(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_30(IDS));
}
template <std::size_t N> requires (N == 31) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_31(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_31(IDS));
}
template <std::size_t N> requires (N == 32) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_32(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_32(IDS));
}
template <std::size_t N> requires (N == 33) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_33(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_33(IDS));
}
template <std::size_t N> requires (N == 34) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_34(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_34(IDS));
}
template <std::size_t N> requires (N == 35) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_35(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_35(IDS));
}
template <std::size_t N> requires (N == 36) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_36(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_36(IDS));
}
template <std::size_t N> requires (N == 37) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_37(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_37(IDS));
}
template <std::size_t N> requires (N == 38) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_38(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_38(IDS));
}
template <std::size_t N> requires (N == 39) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_39(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_39(IDS));
}
template <std::size_t N> requires (N == 40) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_40(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_40(IDS));
}
template <std::size_t N> requires (N == 41) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_41(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_41(IDS));
}
template <std::size_t N> requires (N == 42) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_42(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_42(IDS));
}
template <std::size_t N> requires (N == 43) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_43(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_43(IDS));
}
template <std::size_t N> requires (N == 44) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_44(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_44(IDS));
}
template <std::size_t N> requires (N == 45) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_45(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_45(IDS));
}
template <std::size_t N> requires (N == 46) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_46(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_46(IDS));
}
template <std::size_t N> requires (N == 47) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_47(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_47(IDS));
}
template <std::size_t N> requires (N == 48) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_48(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_48(IDS));
}
template <std::size_t N> requires (N == 49) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_49(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_49(IDS));
}
template <std::size_t N> requires (N == 50) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_50(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_50(IDS));
}
template <std::size_t N> requires (N == 51) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_51(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_51(IDS));
}
template <std::size_t N> requires (N == 52) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_52(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_52(IDS));
}
template <std::size_t N> requires (N == 53) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_53(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_53(IDS));
}
template <std::size_t N> requires (N == 54) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_54(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_54(IDS));
}
template <std::size_t N> requires (N == 55) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_55(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_55(IDS));
}
template <std::size_t N> requires (N == 56) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_56(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_56(IDS));
}
template <std::size_t N> requires (N == 57) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_57(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_57(IDS));
}
template <std::size_t N> requires (N == 58) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_58(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_58(IDS));
}
template <std::size_t N> requires (N == 59) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_59(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_59(IDS));
}
template <std::size_t N> requires (N == 60) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_60(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_60(IDS));
}
template <std::size_t N> requires (N == 61) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_61(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_61(IDS));
}
template <std::size_t N> requires (N == 62) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_62(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_62(IDS));
}
template <std::size_t N> requires (N == 63) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_63(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_63(IDS));
}
template <std::size_t N> requires (N == 64) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_64(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_64(IDS));
}
template <std::size_t N> requires (N == 65) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_65(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_65(IDS));
}
template <std::size_t N> requires (N == 66) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_66(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_66(IDS));
}
template <std::size_t N> requires (N == 67) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_67(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_67(IDS));
}
template <std::size_t N> requires (N == 68) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_68(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_68(IDS));
}
template <std::size_t N> requires (N == 69) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_69(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_69(IDS));
}
template <std::size_t N> requires (N == 70) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_70(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_70(IDS));
}
template <std::size_t N> requires (N == 71) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_71(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_71(IDS));
}
template <std::size_t N> requires (N == 72) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_72(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_72(IDS));
}
template <std::size_t N> requires (N == 73) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_73(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_73(IDS));
}
template <std::size_t N> requires (N == 74) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_74(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_74(IDS));
}
template <std::size_t N> requires (N == 75) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_75(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_75(IDS));
}
template <std::size_t N> requires (N == 76) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_76(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_76(IDS));
}
template <std::size_t N> requires (N == 77) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_77(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_77(IDS));
}
template <std::size_t N> requires (N == 78) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_78(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_78(IDS));
}
template <std::size_t N> requires (N == 79) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_79(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_79(IDS));
}
template <std::size_t N> requires (N == 80) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_80(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_80(IDS));
}
template <std::size_t N> requires (N == 81) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_81(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_81(IDS));
}
template <std::size_t N> requires (N == 82) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_82(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_82(IDS));
}
template <std::size_t N> requires (N == 83) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_83(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_83(IDS));
}
template <std::size_t N> requires (N == 84) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_84(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_84(IDS));
}
template <std::size_t N> requires (N == 85) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_85(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_85(IDS));
}
template <std::size_t N> requires (N == 86) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_86(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_86(IDS));
}
template <std::size_t N> requires (N == 87) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_87(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_87(IDS));
}
template <std::size_t N> requires (N == 88) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_88(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_88(IDS));
}
template <std::size_t N> requires (N == 89) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_89(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_89(IDS));
}
template <std::size_t N> requires (N == 90) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_90(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_90(IDS));
}
template <std::size_t N> requires (N == 91) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_91(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_91(IDS));
}
template <std::size_t N> requires (N == 92) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_92(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_92(IDS));
}
template <std::size_t N> requires (N == 93) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_93(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_93(IDS));
}
template <std::size_t N> requires (N == 94) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_94(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_94(IDS));
}
template <std::size_t N> requires (N == 95) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_95(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_95(IDS));
}
template <std::size_t N> requires (N == 96) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_96(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_96(IDS));
}
template <std::size_t N> requires (N == 97) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_97(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_97(IDS));
}
template <std::size_t N> requires (N == 98) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_98(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_98(IDS));
}
template <std::size_t N> requires (N == 99) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_99(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_99(IDS));
}
template <std::size_t N> requires (N == 100) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_100(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_100(IDS));
}
template <std::size_t N> requires (N == 101) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_101(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_101(IDS));
}
template <std::size_t N> requires (N == 102) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_102(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_102(IDS));
}
template <std::size_t N> requires (N == 103) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_103(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_103(IDS));
}
template <std::size_t N> requires (N == 104) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_104(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_104(IDS));
}
template <std::size_t N> requires (N == 105) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_105(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_105(IDS));
}
template <std::size_t N> requires (N == 106) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_106(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_106(IDS));
}
template <std::size_t N> requires (N == 107) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_107(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_107(IDS));
}
template <std::size_t N> requires (N == 108) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_108(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_108(IDS));
}
template <std::size_t N> requires (N == 109) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_109(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_109(IDS));
}
template <std::size_t N> requires (N == 110) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_110(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_110(IDS));
}
template <std::size_t N> requires (N == 111) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_111(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_111(IDS));
}
template <std::size_t N> requires (N == 112) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_112(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_112(IDS));
}
template <std::size_t N> requires (N == 113) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_113(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_113(IDS));
}
template <std::size_t N> requires (N == 114) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_114(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_114(IDS));
}
template <std::size_t N> requires (N == 115) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_115(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_115(IDS));
}
template <std::size_t N> requires (N == 116) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_116(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_116(IDS));
}
template <std::size_t N> requires (N == 117) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_117(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_117(IDS));
}
template <std::size_t N> requires (N == 118) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_118(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_118(IDS));
}
template <std::size_t N> requires (N == 119) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_119(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_119(IDS));
}
template <std::size_t N> requires (N == 120) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_120(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_120(IDS));
}
template <std::size_t N> requires (N == 121) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_121(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_121(IDS));
}
template <std::size_t N> requires (N == 122) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_122(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_122(IDS));
}
template <std::size_t N> requires (N == 123) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_123(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_123(IDS));
}
template <std::size_t N> requires (N == 124) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_124(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_124(IDS));
}
template <std::size_t N> requires (N == 125) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_125(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_125(IDS));
}
template <std::size_t N> requires (N == 126) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_126(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_126(IDS));
}
template <std::size_t N> requires (N == 127) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_127(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_127(IDS));
}
template <std::size_t N> requires (N == 128) // NOLINT
 constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
	auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_128(IDS) ] = value;
	return fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_128(IDS));
}
#undef IDS
    #pragma endregion
#pragma region element<N, T>
    #define IDS(EXPR) EXPR
    #define DECLTYPE_IDS(EXPR) decltype(EXPR)
template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 1)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_1(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_1(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_1(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 2)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_2(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_2(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_2(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 3)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS) ] = declval<T&>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_3(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_3(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_3(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 4)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_4(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_4(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_4(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 5)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_5(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_5(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_5(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 6)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_6(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_6(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_6(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 7)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_7(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_7(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_7(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 8)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_8(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_8(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_8(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 9)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_9(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_9(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_9(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 10)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_10(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_10(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_10(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 11)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_11(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_11(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_11(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_11(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 12)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_12(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_12(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_12(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_12(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 13)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_13(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_13(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_13(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_13(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 14)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_14(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_14(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_14(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_14(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 15)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_15(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_15(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_15(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_15(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 16)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_16(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_16(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_16(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_16(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 17)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_17(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_17(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_17(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_17(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 18)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_18(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_18(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_18(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_18(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 19)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_19(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_19(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_19(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_19(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 20)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_20(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_20(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_20(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_20(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 21)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_21(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_21(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_21(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_21(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 22)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_22(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_22(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_22(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_22(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 23)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_23(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_23(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_23(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_23(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 24)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_24(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_24(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_24(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_24(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 25)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_25(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_25(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_25(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_25(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 26)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_26(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_26(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_26(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_26(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 27)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_27(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_27(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_27(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_27(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 28)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_28(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_28(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_28(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_28(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 29)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_29(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_29(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_29(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_29(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 30)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_30(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_30(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_30(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_30(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 31)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_31(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_31(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_31(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_31(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 32)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_32(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_32(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_32(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_32(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 33)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_33(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_33(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_33(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_33(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 34)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_34(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_34(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_34(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_34(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 35)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_35(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_35(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_35(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_35(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 36)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_36(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_36(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_36(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_36(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 37)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_37(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_37(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_37(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_37(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 38)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_38(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_38(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_38(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_38(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 39)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_39(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_39(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_39(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_39(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 40)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_40(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_40(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_40(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_40(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 41)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_41(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_41(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_41(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_41(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 42)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_42(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_42(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_42(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_42(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 43)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_43(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_43(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_43(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_43(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 44)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_44(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_44(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_44(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_44(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 45)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_45(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_45(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_45(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_45(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 46)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_46(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_46(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_46(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_46(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 47)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_47(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_47(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_47(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_47(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 48)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_48(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_48(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_48(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_48(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 49)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_49(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_49(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_49(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_49(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 50)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_50(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_50(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_50(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_50(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 51)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_51(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_51(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_51(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_51(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 52)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_52(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_52(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_52(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_52(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 53)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_53(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_53(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_53(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_53(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 54)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_54(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_54(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_54(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_54(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 55)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_55(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_55(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_55(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_55(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 56)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_56(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_56(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_56(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_56(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 57)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_57(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_57(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_57(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_57(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 58)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_58(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_58(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_58(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_58(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 59)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_59(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_59(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_59(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_59(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 60)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_60(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_60(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_60(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_60(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 61)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_61(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_61(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_61(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_61(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 62)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_62(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_62(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_62(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_62(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 63)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_63(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_63(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_63(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_63(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 64)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_64(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_64(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_64(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_64(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 65)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_65(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_65(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_65(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_65(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 66)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_66(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_66(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_66(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_66(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 67)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_67(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_67(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_67(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_67(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 68)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_68(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_68(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_68(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_68(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 69)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_69(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_69(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_69(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_69(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 70)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_70(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_70(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_70(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_70(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 71)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_71(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_71(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_71(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_71(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 72)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_72(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_72(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_72(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_72(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 73)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_73(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_73(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_73(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_73(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 74)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_74(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_74(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_74(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_74(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 75)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_75(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_75(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_75(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_75(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 76)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_76(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_76(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_76(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_76(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 77)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_77(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_77(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_77(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_77(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 78)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_78(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_78(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_78(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_78(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 79)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_79(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_79(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_79(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_79(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 80)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_80(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_80(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_80(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_80(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 81)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_81(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_81(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_81(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_81(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 82)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_82(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_82(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_82(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_82(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 83)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_83(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_83(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_83(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_83(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 84)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_84(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_84(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_84(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_84(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 85)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_85(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_85(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_85(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_85(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 86)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_86(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_86(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_86(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_86(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 87)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_87(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_87(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_87(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_87(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 88)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_88(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_88(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_88(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_88(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 89)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_89(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_89(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_89(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_89(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 90)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_90(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_90(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_90(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_90(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 91)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_91(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_91(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_91(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_91(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 92)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_92(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_92(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_92(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_92(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 93)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_93(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_93(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_93(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_93(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 94)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_94(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_94(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_94(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_94(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 95)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_95(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_95(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_95(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_95(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 96)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_96(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_96(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_96(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_96(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 97)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_97(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_97(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_97(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_97(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 98)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_98(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_98(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_98(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_98(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 99)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_99(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_99(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_99(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_99(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 100)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_100(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_100(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_100(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_100(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 101)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_101(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_101(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_101(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_101(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 102)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_102(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_102(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_102(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_102(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 103)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_103(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_103(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_103(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_103(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 104)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_104(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_104(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_104(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_104(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 105)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_105(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_105(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_105(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_105(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 106)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_106(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_106(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_106(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_106(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 107)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_107(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_107(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_107(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_107(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 108)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_108(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_108(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_108(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_108(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 109)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_109(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_109(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_109(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_109(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 110)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_110(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_110(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_110(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_110(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 111)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_111(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_111(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_111(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_111(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 112)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_112(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_112(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_112(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_112(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 113)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_113(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_113(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_113(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_113(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 114)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_114(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_114(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_114(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_114(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 115)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_115(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_115(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_115(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_115(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 116)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_116(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_116(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_116(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_116(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 117)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_117(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_117(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_117(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_117(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 118)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_118(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_118(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_118(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_118(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 119)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_119(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_119(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_119(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_119(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 120)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_120(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_120(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_120(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_120(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 121)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_121(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_121(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_121(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_121(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 122)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_122(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_122(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_122(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_122(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 123)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_123(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_123(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_123(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_123(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 124)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_124(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_124(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_124(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_124(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 125)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_125(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_125(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_125(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_125(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 126)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_126(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_126(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_126(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_126(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 127)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_127(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_127(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_127(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_127(DECLTYPE_IDS)>
            >{};
        }()){};
    template <std::size_t N, concepts::aggregate T>

        requires (fields_count<T> == 128)

        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_128(IDS) ] = std::declval<T>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_128(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_128(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_128(DECLTYPE_IDS)>
            >{};
        }()){};
    #undef IDS
     #undef DECLTYPE_IDS
     #pragma endregion
// END OF GENERATED CONTENT
#pragma endregion

    template <std::size_t>
    constexpr auto as_tuple_impl(concepts::aggregate auto && value) {
        static_assert([](){ return false; }(), "[srl] exceed maxmimum members count");
    }
}
namespace csl::ag {

    constexpr auto as_tuple(auto && value) {
        using type = std::remove_cvref_t<decltype(value)>;
        return details::as_tuple_impl<details::fields_count<type>>(std::forward<decltype(value)>(value));
    }

	// tuple-like
	// TODO : std::get/std::tuple_size/std::tuple_element
	// size
    template <concepts::aggregate T>
    using size = std::integral_constant<std::size_t, details::fields_count<T>>;
	// get
    template <std::size_t N>
    constexpr decltype(auto) get(concepts::aggregate auto && value) {
        return ::std::get<N>(as_tuple(std::forward<decltype(value)>(value)));
    }
	// element
    template <std::size_t N, concepts::aggregate T>
    using element = typename details::element<N, T>;
}
