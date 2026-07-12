#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#include <csl/ag/formatting/typeinfo.hpp>
#include <csl/ensure.hpp>
#include <csl/functional.hpp>
#include <csl/mp.hpp>
#include <csl/typeinfo.hpp>
#include <csl/wf.hpp>

#include <format>

struct point { int x, y; };
static_assert(csl::ag::size_v<point> == 2);

using id = csl::ensure::strong_type<int, struct id_tag>;
static_assert(std::is_constructible_v<id, int>);

static_assert(std::is_same_v<
    csl::functional::function_trait<bool(int, char)>::result_type,
    bool
>);

static_assert(std::is_same_v<
    std::tuple_element_t<0, csl::mp::tuple<int, char>>,
    int
>);

static_assert(csl::typeinfo::type_name_v<int> == "int");

// typeinfo bridge: csl::ag::io::type_name is csl::typeinfo-backed (compile-time)
static_assert(csl::ag::io::type_name_v<int> == "int");

auto main() -> int {
    using namespace csl::ag::io;
    return std::format("{}", point{ .x = 1, .y = 2 } | typenamed) == "{int: 1, int: 2}" ? 0 : 1;
}
