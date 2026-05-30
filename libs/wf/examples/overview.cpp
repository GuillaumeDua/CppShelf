#include <csl/wf.hpp>

#include <cassert>
#include <string>

using namespace csl::wf::literals;  // _times

// --- type traits ---

using F = decltype([]<typename T>(T v) { return v; });

static_assert(    csl::wf::mp::is_invocable_v<F, csl::wf::mp::ttps<int>, int>);
static_assert(not csl::wf::mp::is_invocable_v<F, csl::wf::mp::ttps<int>, float>);
static_assert(    csl::wf::mp::is_invocable_with_v<F, csl::wf::mp::ttps<int>, csl::wf::mp::args<int>>);

auto main(int, char*[]) -> int
{
    // --- invoke with template parameters ---
    auto typed = []<typename T>(T v) { return v * 2; };
    assert(csl::wf::invoke<int>(typed, 21) == 42); // NOLINT(*-assert)

    // --- apply / apply_before / apply_after ---
    auto add = [](int a, int b) { return a + b; };
    auto args = std::tuple{10, 32};
    assert(csl::wf::apply(add, args) == 42);           // NOLINT(*-assert)
    assert(csl::wf::apply_after (add, std::tuple{32}, 10) == 42); // NOLINT(*-assert)
    assert(csl::wf::apply_before(add, std::tuple{10}, 32) == 42); // NOLINT(*-assert)

    // --- bind_front / bind_back ---
    auto add10 = csl::wf::bind_front(add, 10);
    assert(add10(32) == 42); // NOLINT(*-assert)

    auto add10_back = csl::wf::bind_back(add, 10);
    assert(add10_back(32) == 42); // NOLINT(*-assert)

    // --- function_ref: non-owning, rebindable ---
    auto square = [](int x) { return x * x; };
    csl::wf::function_ref ref{square};
    assert(ref(6) == 36); // NOLINT(*-assert)

    // --- route: pipeline composition ---
    auto pipeline = csl::wf::make_continuation(
        [](int x)         { return x * 2; },
        [](int x)         { return std::to_string(x); },
        [](std::string s) { return s + "!"; }
    );
    assert(pipeline(21) == "42!"); // NOLINT(*-assert)

    // --- eDSL operators ---
    using namespace csl::wf::operators;

    auto process =
        [](int x) { return x + 1; }
        >>= [](int x) { return x * 10; };
    assert(process(3) == 40); // NOLINT(*-assert)

    auto handle = [](int x)    { return x > 0; }
                | [](float x)  { return x > 0.f; };
    assert(handle(1));    // NOLINT(*-assert)
    assert(handle(1.f));  // NOLINT(*-assert)

    // --- repeater ---
    int count = 0;
    auto tick = [&count] { ++count; } * 3_times;
    tick();
    assert(count == 3); // NOLINT(*-assert)

    // repeater flattening
    auto double_repeat = ([&count]{ ++count; } * 2_times) * 3_times;
    count = 0;
    double_repeat();
    assert(count == 6); // NOLINT(*-assert)

    // --- function_view via operator| ---
    auto f = [](int x) { return x - 1; };
    auto view = f | csl::wf::operators::view;
    assert(view(43) == 42); // NOLINT(*-assert)
}
