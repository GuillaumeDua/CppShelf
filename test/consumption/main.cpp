#include <csl/ag.hpp>
#include <csl/ensure.hpp>
#include <csl/functional.hpp>
#include <csl/mp.hpp>
#include <csl/typeinfo.hpp>
#include <csl/wf.hpp>

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

auto main() -> int {}
