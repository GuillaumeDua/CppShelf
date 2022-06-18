#include <csl/ag.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

//  TODO(Guss) : (std|fmt)::formatter
//  TODO(Guss) : cvref-correctness (of printed values)
namespace csl::ag::io {
    //  For GCC [10.3 .. 12.1] : Can't use the following synthax anymore (constraint depends on itself)
    //  (might be same issue as https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99599)
    //      std::ostream & operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      requires (not ostream_shiftable<decltype(value)>::value)
    //  nor delayed adl :
    //      std::ostream & details::ostream_shift(std::ostream & os, csl::ag::concepts::aggregate auto const& value)
    //      requires (not ostream_shiftable<decltype(value)>::value); // never defined
    //
    //      auto operator<<(std::ostream & os, csl::ag::concepts::aggregate auto && value)
    //      -> decltype(details::ostream_shift(os, value))
    //
    // Warning : csl::ag makes aggregate type tuplelike

    auto & operator<<(const gcl::io::indented_ostream os, csl::ag::concepts::structured_bindable auto && value)
    requires (not std::is_array_v<std::remove_cvref_t<decltype(value)>>) // temporary quickfix
    {
        using value_type = std::remove_cvref_t<decltype(value)>;

        constexpr auto size = []() constexpr { // work-around for ADL issue
            if constexpr (csl::ag::concepts::tuplelike<value_type>)
                return std::tuple_size_v<value_type>;
            else if constexpr (csl::ag::concepts::aggregate<value_type>)
                return csl::ag::size_v<value_type>;
            else
                static_assert(sizeof(value_type) and false, "csl::ag::print : invalid type"); // NOLINT
        }();

        using namespace gcl::io;
        os << gcl::cx::type_name_v<decltype(value)> << " : {\n";

        const auto print_value = [&]<size_t index>(){
            os << indent(rel(1)) << '[' << index << "] ";
            auto && element_value = std::get<index>(std::forward<decltype(value)>(value));
            using element_value_type = decltype(element_value);
            if constexpr (not csl::ag::concepts::structured_bindable<element_value_type>)
                os << gcl::cx::type_name_v<element_value_type> << " : ";
            os << element_value << '\n';
        };

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((print_value.template operator()<indexes>()), ...);  
        }(std::make_index_sequence<size>{});

        os << indent << "}";
        return os.bounded_to();
    }
}

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

    // {
    //     auto value = toto{ 42, 'a' };
    //     [[maybe_unused]] auto && [ v0, v1 ] = value;
    //     print(value);
    // }

    // auto value = toto{ 42, 'A' }; // NOLINT
    // auto as_tuple = csl::ag::as_tuple(value); // WTF not a constant expression ???

    // static_assert(std::same_as<
    //     int&,
    //     std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    // >);
    // static_assert(std::same_as<
    //     char&,
    //     std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    // >);

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
