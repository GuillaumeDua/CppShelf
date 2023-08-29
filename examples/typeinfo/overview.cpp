#include <csl/typeinfo.hpp>

namespace demo {
    enum my_enum{ a, b, c };
    struct my_type{
        using my_nested_type = int;
        constexpr static auto my_nested_value = 42;
    };

    template <typename T>
    struct my_templated_type {
        using type = T;
    };
}

// type_name(_v)
static_assert(csl::typeinfo::type_name_v<int> == "int");
static_assert(csl::typeinfo::type_name_v<demo::my_enum> == "demo::my_enum");
static_assert(csl::typeinfo::type_name_v<demo::my_type> == "demo::my_type");
static_assert(csl::typeinfo::type_name_v<demo::my_templated_type<int>> == "demo::my_templated_type<int>");
static_assert(csl::typeinfo::type_name_v<demo::my_type::my_nested_type> == "int");
static_assert(csl::typeinfo::type_name_v<decltype(demo::my_type::my_nested_value)> == "const int");

// value_name(_v)
static_assert(csl::typeinfo::value_name_v<demo::my_type::my_nested_value> == "42");
static_assert(csl::typeinfo::value_name_v<demo::my_enum::a> == "demo::a");

#include <iostream>
#include <string>
auto main(int, char*argv[]) -> int {
    std::cout << csl::typeinfo::type_name_v<std::string> << '\n';
}
