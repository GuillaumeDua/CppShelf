#include <csl/wf.hpp>

#include <cassert>
#include <string>

// csl::wf: composable, type-safe callable invocation.

// --- compile-time invocability ---
using typed_identity = decltype([]<typename T>(T v) { return v; });

static_assert(    csl::wf::mp::is_invocable_v<typed_identity, csl::wf::mp::ttps<int>, int        >);
static_assert(    csl::wf::mp::is_invocable_v<typed_identity, csl::wf::mp::ttps<int>, float      >); // implicit convertion
static_assert(not csl::wf::mp::is_invocable_v<typed_identity, csl::wf::mp::ttps<int>, std::string>);
static_assert(    csl::wf::mp::is_invocable_with_v<
    typed_identity,
    csl::wf::mp::ttps<int>,
    csl::wf::mp::args<int>
>);

auto main(int, char*[]) -> int
{
    // --- invoke with explicit template parameters ---
    // std::invoke cannot deduce the lambda's ttp T, csl::wf::invoke can.
    auto scale = []<typename T>(T v) { return v * T{2}; };
    assert(csl::wf::invoke<int>(scale, 21) == 42); // NOLINT(*-assert)

    // --- tuple unpacking ---
    // apply_before : f(tuple..., extras...) - packed args prepended
    // apply_after  : f(extras..., tuple...) - packed args appended
    auto add = [](int a, int b) { return a + b; };
    assert(csl::wf::apply       (add, std::tuple{10, 32})    == 42); // NOLINT(*-assert)
    assert(csl::wf::apply_before(add, std::tuple{10}, 32)    == 42); // NOLINT(*-assert)
    assert(csl::wf::apply_after (add, std::tuple{32}, 10)    == 42); // NOLINT(*-assert)

    // --- partial application ---
    auto add10   = csl::wf::bind_front(add, 10);
    auto add10_b = csl::wf::bind_back (add, 10);
    assert(add10  (32) == 42); // NOLINT(*-assert)
    assert(add10_b(32) == 42); // NOLINT(*-assert)

    // --- pipeline: make_continuation chains callables left-to-right ---
    // invoke the result before `using namespace csl::wf::operators`:
    // the unconstrained >>= from that namespace conflicts with chain_invoke's internal fold.
    auto pipeline = csl::wf::make_continuation(
        [](int x) { return x * 2; },
        [](int x) { return std::to_string(x) + "!"; }
    );
    assert(pipeline(21) == "42!"); // NOLINT(*-assert)

    // --- eDSL operators ---
    using namespace csl::wf::operators;

    // >>= : operator form of make_continuation (type-stable pipelines)
    auto process =
        [](int x) { return x * 10; }
    >>= [](int x) { return x + 2; }
    >>= [](int x) { return std::to_string(x); };
    assert(process(4) == std::string{"42"}); // NOLINT(*-assert)

    // | : overload set - dispatches on argument type at call time
    auto is_positive =
        [](int   x) { return x > 0; }
    |   [](float x) { return x > 0.F; };
    assert(is_positive(1));   // NOLINT(*-assert)
    assert(is_positive(1.F)); // NOLINT(*-assert)

    // * N_times : invoke N times per call
    using namespace csl::wf::literals;

    int count = 0;
    ([&count]{ ++count; } * 5_times)();
    assert(count == 5); // NOLINT(*-assert)

    // repeaters compose: (2_times) * (3_times) = 6 calls total
    count = 0;
    ([&count]{ ++count; } * 2_times * 3_times)();
    assert(count == 6); // NOLINT(*-assert)

    // --- non-owning callable wrappers ---
    // ref  : pointer-like (stores address, rebindable between same-type instances)
    // view : reference-like (stores reference, default-constructible)
    auto f   = [](int x) { return x - 1; };
    auto fref = f | ref;
    auto fvw  = f | csl::wf::operators::view; // qualified: avoids shadowing the imported 'view' tag
    assert(fref(43) == 42); // NOLINT(*-assert)
    assert(fvw (43) == 42); // NOLINT(*-assert)
}
