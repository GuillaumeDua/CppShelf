I want to improve my `.clang-format` file, so clang-format then produces better code formatting.

Acceptance criteria: should not change much, when compared to what I have now in `includes/mp/csl/mp.hpp`.

- 1: namespace should start with an empty line.
- 2: break long/complexe lines of C++ templates ?

    for instance `using container = std::unordered_map<std::basic_string<char, std::allocator<char>>, std::array<int, 2>>` could be:

    using container = std::unordered_map<
        std::basic_string<
            char,
            std::allocator<char>
        >,
        std::array<int, 2>
    >;

    for instance, I have:
        struct make_tuple_storage<std::index_sequence<indexes...>, Ts...> : type_identity<typename mp::details::tuple_storage<mp::details::tuple_member<indexes, Ts>...>> {};
    what I want:
        template <std::size_t... indexes, typename... Ts>
        struct make_tuple_storage<std::index_sequence<indexes...>, Ts...> : type_identity<
            typename mp::details::tuple_storage<
                mp::details::tuple_member<indexes, Ts>...
            >
        >{};

    for instance, I have:
        template <
            concepts::tuple T,
            concepts::tuple U,
            template <typename> class TQual,
            template <typename> class UQual>
        struct tuple_common_reference;

        template <
            typename... Ts,
            typename... Us,
            template <typename> class TQual,
            template <typename> class UQual>
        requires(sizeof...(Ts) == sizeof...(Us))
            and (true and ... and std::common_reference_with<TQual<Ts>, UQual<Us>>)
        struct tuple_common_reference<
            tuple<Ts...>,
            tuple<Us...>,
            TQual,
            UQual> {
            using type = tuple<
                std::common_reference_t<
                    TQual<Ts>,
                    UQual<Us>>...>;
        };
    what I want:
        template <
            typename ... Ts,
            typename ... Us,
            template <typename> class TQual,
            template <typename> class UQual
        >
        requires
                (sizeof...(Ts) == sizeof...(Us))
            and (true and ... and std::common_reference_with<TQual<Ts>, UQual<Us>>)
        struct tuple_common_reference<
            tuple<Ts...>, tuple<Us...>,
            TQual, UQual
        >
        {
            using type = tuple<
                std::common_reference_t<
                    TQual<Ts>,
                    UQual<Us>
                >...
            >;
        };

    What I have:
        template <
            csl::mp::concepts::tuple_like          shape,
            csl::mp::concepts::tuple_like          elements_source,
            csl::mp::seq::concepts::index_sequence index_sequence = std::make_index_sequence<
                std::tuple_size_v<std::remove_cvref_t<elements_source>>>>
        struct rebind_N_elements;
    What I want:
        template <
            csl::mp::concepts::tuple_like          shape,
            csl::mp::concepts::tuple_like          elements_source,
            csl::mp::seq::concepts::index_sequence index_sequence = std::make_index_sequence<
                std::tuple_size_v<std::remove_cvref_t<elements_source>>
            >
        >
        struct rebind_N_elements;

    What I have:
        template <csl::mp::concepts::tuple_like shape, csl::mp::concepts::tuple_like elements_source>
        struct rebind_elements : rebind_N_elements<shape, elements_source, std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<elements_source>>>> {};
    What I want:
        template <csl::mp::concepts::tuple_like shape, csl::mp::concepts::tuple_like elements_source>
        struct rebind_elements : rebind_N_elements<
            shape,
            elements_source,
            std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<elements_source>>>
        >{};

- 3: align concepts clause, one by line, breaking after =, and breaking before logical keywords.

     for instance `template <typename T> concept some_concept = true and (not false) and int{} == 0;
     should be:
        template <typename T>
        concept some_concept =
                true
            and (not false)
            and int{} == 0
        ;
     for instance, I have:
        template <typename T, std::size_t N>
        concept tuple_element = requires { std::tuple_size<T>{}; }
            and std::tuple_size_v<T> > N and requires(T t) {
                    typename std::tuple_element_t<N, std::remove_const_t<T>>;
                    { get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
                };
     what I want:
        	template <typename T, std::size_t N>
    concept tuple_element =
            requires { std::tuple_size<T>{}; }
        and std::tuple_size_v<T> > N
        and requires(T t) {
            typename std::tuple_element_t<N, std::remove_const_t<T>>;
            { get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T>&>;
        }
    ;

- 4: static_assert clauses

    currently I have: 
        static_assert(std::is_same_v<
                csl::mp::tuple<int &&, const char &>,
                decltype(csl::mp::functions::forward_as_tuple(42, c))>);
    what I want:
        static_assert(std::is_same_v<
            csl::mp::tuple<int &&, const char &>,
            decltype(csl::mp::functions::forward_as_tuple(42, c))
        >);
- 5: don't add a space between struct or class name and {}
    I have: struct A: std::type_identity<T> {};
    I want: struct A: std::type_identity<T>{};
- 6: break long inheritance lines
    I have: struct at<index, std::integer_sequence<T, values...>>: std::integral_constant<T, std::get<index>(to_tuplelike_v<std::integer_sequence<T, values...>>)> {};
    I want:
        struct at<index, std::integer_sequence<T, values...>>: std::integral_constant<
            T,
            std::get<index>(to_tuplelike_v<std::integer_sequence<T, values...>>)
        >{};
- 7: break before noexcept and require clause

    I have:
        constexpr tuple() noexcept((true and ... and std::is_nothrow_constructible_v<Ts>))
        requires(true and ... and std::is_default_constructible_v<Ts>)
        = default;
    I want:
        constexpr tuple()
        noexcept ((true and ... and std::is_nothrow_constructible_v<Ts>))
        requires  (true and ... and std::is_default_constructible_v<Ts>)
        = default;

    I have:
        template <typename... Us>
        constexpr explicit(not(true and ... and std::convertible_to<Ts, Us &&>)) tuple(Us &&... args) // NOLINT(*-forward)
            noexcept((std::is_nothrow_constructible_v<Ts, Us &&> and ...))
        requires(sizeof...(Ts) not_eq 0) // disambiguate with default constructor
            and (sizeof...(Ts) == sizeof...(Us))
            and (std::constructible_from<Ts, Us &&> and ...)
            : storage{csl_fwd(args)...} {}
    I want:
        template <typename... Us>
        explicit(not(
            true and ... and std::convertible_to<Ts, Us &&>
        ))
        constexpr tuple(Us &&... args) // NOLINT(*-forward)
        noexcept((std::is_nothrow_constructible_v<Ts, Us &&> and ...))
        requires
                (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
            and (sizeof...(Ts) == sizeof...(Us))
            and (std::constructible_from<Ts, Us &&> and ...)
        : storage{ csl_fwd(args)... }
        {}

    What I have:

        constexpr auto for_each(csl::mp::concepts::tuple_like auto && value, auto && f)noexcept(csl::mp::concepts::can_nothrow_for_each<decltype(value), decltype(f)>)
        requires csl::mp::concepts::can_for_each<decltype(value), decltype(f)>
        {
            constexpr auto size = std::tuple_size_v<std::remove_cvref_t<decltype(value)>>;
            [&]<std::size_t... indexes>(std::index_sequence<indexes...>) {
                ((
                    std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value)))
                ),
                ...);
            }(std::make_index_sequence<size>{});
            return f;
        }
    What I want:
        constexpr auto for_each(csl::mp::concepts::tuple_like auto && value, auto && f)
        noexcept(csl::mp::concepts::can_nothrow_for_each<decltype(value), decltype(f)>)
        requires csl::mp::concepts::can_for_each<decltype(value), decltype(f)>
        {
            constexpr auto size = std::tuple_size_v<std::remove_cvref_t<decltype(value)>>;
            [&]<std::size_t... indexes>(std::index_sequence<indexes...>) {
                (
                    (
                        std::invoke(csl_fwd(f), get<indexes>(csl_fwd(value)))
                    ),
                    ...
                );
            }(std::make_index_sequence<size>{});
            return f;
        }

- 8: add an empty line after #pragma region. add an empty line before #pragma endregion 
- 9: indent preprocessor directives

    What I have:
         #if defined(__cpp_explicit_this_parameter) \
            and __cpp_explicit_this_parameter >= 202110L
    What I want:
            #if defined(__cpp_explicit_this_parameter) \
                    and __cpp_explicit_this_parameter >= 202110L
- 10: break before IILE in type definition

    What I have:
        template <concepts::tuple_like T>
        struct is_homogeneous<T> : std::bool_constant<[]<std::size_t... indexes>(std::index_sequence<indexes...>) {
            return (true and ... and std::same_as<csl::mp::element<0, T>, csl::mp::element<indexes, T>>);
        }(std::make_index_sequence<csl::mp::size_v<T>>{})> {};
    What I want:
        template <concepts::tuple_like T>
        struct is_homogeneous<T> : std::bool_constant<
            []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return (true and ... and std::same_as<element<0, T>, element<indexes, T>>);
            }(std::make_index_sequence<size_v<T>>{})
        >{};
- 11: break before IILE in concepts definition

    What I have:
        template <typename T, typename F>
        concept can_for_each = concepts::tuple_like<T> and[]<std::size_t... indexes>(std::index_sequence<indexes...>) {
            return (true and ... and std::is_invocable_v<F, decltype(get<indexes>(std::declval<T>()))>);
        }
        (std::make_index_sequence<csl::mp::size_v<std::remove_cvref_t<T>>>{});
    What I want:
        template <typename T, typename F>
        concept can_for_each = concepts::tuple_like<T>
            and []<std::size_t... indexes>(std::index_sequence<indexes...>) {
                return (true and ... and std::is_invocable_v<F, decltype(get<indexes>(std::declval<T>()))>);
            }(std::make_index_sequence<csl::mp::size_v<std::remove_cvref_t<T>>>{});

- 12: break long static_assert lines

    What I have:
        static_assert(std::same_as<
                csl::mp::tuple_common_reference_t<
                    lhs_t,
                    rhs_t,
                    std::type_identity_t,
                    std::type_identity_t>,
                csl::mp::tuple<
                    std::common_reference_t<
                        std::tuple_element_t<0, lhs_t>,
                        std::tuple_element_t<0, rhs_t>>,
                    std::common_reference_t<
                        std::tuple_element_t<1, lhs_t>,
                        std::tuple_element_t<1, rhs_t>>>>);
    What I want:
        static_assert(std::same_as<
            csl::mp::tuple_common_reference_t<
                lhs_t,
                rhs_t,
                std::type_identity_t,
                std::type_identity_t
            >,
            csl::mp::tuple<
                std::common_reference_t<
                    std::tuple_element_t<0, lhs_t>,
                    std::tuple_element_t<0, rhs_t>
                >,
                std::common_reference_t<
                    std::tuple_element_t<1, lhs_t>,
                    std::tuple_element_t<1, rhs_t>
                >
            >
        >);

        Keeping the same indentation logic for angle brackets than braces.

    What I have:
        static_assert(std::common_reference_with<
                lhs_t,
                rhs_t>);
    What I want:
        static_assert(std::common_reference_with<lhs_t, rhs_t>);
