#include <csl/ag.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

struct type_0{ int i = 0; char c = 'a'; };
struct type_1{ char c = 'c'; type_0 t; };
struct type_2{ bool b = true; type_1 t; std::tuple<int, char> tu = { 2, 'b'}; std::array<char, 3> a = {'a', 'b', 'c'}; std::pair<int, int> p = { 42, 43 }; }; // NOLINT
struct type_3{ int i = 0; char c = 'a'; };
struct type_4{ const int i = 0; char & c; };

auto & operator<<(std::ostream & os, type_3) { return os << "type_3 : user-defined operator<<(std::ostream&, const T &)\n"; }

auto main() -> int {

    using namespace csl::ag::io;

    char c = 'a';

    std::cout
        << type_0{} << "\n-----\n"
        << type_1{} << "\n-----\n"
        << type_2{} << "\n-----\n"
        << type_3{} << "\n-----\n"
        << type_4{ 42, c } << "\n-----\n"
        // << std::tuple{42, 'a'} << "-----\n"
    ;
    struct A{ int i; float f; };
    struct B {
        A a;
        int i;
        std::string str;
        char && c;
    } v1 { 
        .a = A{ 13, .12f},
        .i = 42, .str = "str", .c = std::move(c)
    };
    std::cout << v1 << '\n'; // const-lvalue-ref

    {   // std::tuple_element_t
        auto value = type_0{ 42, 'A' }; // NOLINT
        [[maybe_unused]] auto && [ v0, v1 ] = value;
        [[maybe_unused]] /*constexpr*/ auto as_tuple = csl::ag::as_tuple_view(value); // WTF not a constant expression ???
        static_assert(std::same_as<
            decltype(as_tuple),
            std::tuple<int&, char&>
        >);

        static_assert(std::same_as<
            int&,
            std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
        >);
        static_assert(std::same_as<
            char&,
            std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
        >);
        static_assert(std::same_as<
            decltype(std::get<0>(as_tuple)),
            std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
        >);
        static_assert(std::same_as<
            decltype(std::get<1>(as_tuple)),
            std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
        >);
        static_assert(std::same_as<
            int &&,
            csl::ag::view_element_t<0, decltype(value)>
        >);
        static_assert(std::same_as<
            char&&,
            csl::ag::view_element_t<1, std::remove_cvref_t<decltype(value)>>
        >);

        static_assert(std::same_as<
            int,
            std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
        >);
        static_assert(std::same_as<
            char,
            std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
        >);
    }
    {   // std::tuple_element_t
        char c = 'c';
        auto value = type_4{ 42, c }; // NOLINT

        static_assert(std::same_as<
            const int,
            std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
        >);
        static_assert(std::same_as<
            char&,
            std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
        >);
    }
    {   // get
        struct A{ int i; float f; };
        auto value = A { .i = 42, .f = 0.13f };

        std::cout << std::get<0>(value) << ", " << std::get<1>(value) << '\n';
        static_assert(std::same_as<
            int &,
            decltype(std::get<0>(value))
        >);
        static_assert(std::same_as<
            float &,
            decltype(std::get<1>(value))
        >);
    }
    {
        struct A{ int i; float f; };
        auto value = A{ .i = 42, .f = 0.13f };

        [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((std::cout << std::get<indexes>(value) << ' '), ...);
        }(std::make_index_sequence<csl::ag::size_v<A>>{});
        std::cout << '\n';
    }
    {   // as_tuple_view ref-qualifiers
        struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };
        int i = 42;

        {   // rvalue
            [[maybe_unused]] auto as_tuple = csl::ag::as_tuple_view(type{ i, i, i, std::move(i) }); // not constexpr yet
            static_assert(std::same_as<
                decltype(as_tuple),
                std::tuple<int&&, int&&, const int&&, int&&>
            >);
        }
        {   // const lvalue
            const auto & value = type{ i, i, i, std::move(i) };
            std::get<1>(csl::ag::as_tuple_view(value)) = 42;
            [[maybe_unused]] auto as_tuple = csl::ag::as_tuple_view(value); // not constexpr yet
            static_assert(std::same_as<
                decltype(as_tuple),
                std::tuple<const int &, int&, const int &, int&>
            >);
        }
    }
    {   // to-tuple (owning)
        struct A{ int i; float f; };
        constexpr auto value = csl::ag::to_tuple(A{ .i = 42, .f = 0.13f });

        [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((std::cout << std::get<indexes>(value) << ' '), ...);
        }(std::make_index_sequence<csl::ag::size_v<A>>{});

        static_assert(std::same_as<
            int,
            std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
        >);
        static_assert(std::same_as<
            float,
            std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
        >);
    }
    {
        struct A{ int & i; float && f; };
        int i = 42; float f = .13f;
        /* not constexpr */ auto value = csl::ag::to_tuple(A{ .i = i, .f = std::move(f) });

        [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((std::cout << std::get<indexes>(value) << ' '), ...);
        }(std::make_index_sequence<csl::ag::size_v<A>>{});

        static_assert(std::same_as<
            int,
            std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
        >);
        static_assert(std::same_as<
            float,
            std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
        >);
    }

    // // /*static_*/assert(csl::ag::get<0>(value) == 42);
    // // /*static_*/assert(csl::ag::get<1>(value) == 'A');

    // assert(
    //     std::addressof(std::get<0>(as_tuple)) ==
    //     std::addressof(value.i)
    // );
    // assert(
    //     std::addressof(std::get<1>(as_tuple)) ==
    //     std::addressof(value.c)
    // );

    // // // print(value);

    // struct qwe{ int i; char & c; bool && b; };
    // // static_assert(std::tuple_size<qwe>::value == 3);
    // static_assert(std::same_as<
    //     int,
    //     csl::ag::element<0, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     char &,
    //     csl::ag::element<1, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     bool &&,
    //     csl::ag::element<2, qwe>::type
    // >);
}
