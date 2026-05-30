#include <csl/ag.hpp>

#include <cstdint>
#include <tuple>

struct point  { int x, y; };
struct rgb    { std::uint8_t r, g, b; };
struct single { float value; };

// size and element type
static_assert(csl::ag::size_v<point>  == 2);
static_assert(csl::ag::size_v<rgb>    == 3);
static_assert(csl::ag::size_v<single> == 1);

static_assert(std::is_same_v<csl::ag::element_t<0, point>, int>);
static_assert(std::is_same_v<csl::ag::element_t<1, point>, int>);
static_assert(std::is_same_v<csl::ag::element_t<2, rgb>,   std::uint8_t>);

// concepts
static_assert(    csl::ag::concepts::aggregate<point>);
static_assert(not csl::ag::concepts::aggregate<std::tuple<int>>);

auto main(int, char*[]) -> int
{
    // --- element access ---
    constexpr point p{ 3, 4 };
    static_assert(csl::ag::get<0>(p) == 3);
    static_assert(csl::ag::get<1>(p) == 4);

    // --- apply: unpack fields into a callable ---
    constexpr auto dist_sq = [](int x, int y) { return x * x + y * y; };
    static_assert(csl::ag::apply(dist_sq, p) == 25);

    // --- for_each: visit every field ---
    constexpr auto channel_sum = [] {
        int s = 0;
        csl::ag::for_each([&s](auto v) { s += static_cast<int>(v); }, rgb{10, 20, 30});
        return s;
    }();
    static_assert(channel_sum == 60);

    // --- for_each_enumerated: field index as NTTP ---
    constexpr auto indexed_ok = [] {
        bool ok = true;
        csl::tuplelike::for_each_enumerated(p, [&ok]<std::size_t I>(auto v) {
            if constexpr (I == 0) ok = ok and (v == 3);
            if constexpr (I == 1) ok = ok and (v == 4);
        });
        return ok;
    }();
    static_assert(indexed_ok);

    // --- for_each_zipped: parallel iteration over multiple tuple-likes ---
    constexpr auto zip_ok = [] {
        int dot = 0;
        csl::ag::for_each_zipped(
            [&dot](auto a, auto b) { dot += a * b; },
            point{1, 2}, point{3, 4}
        );
        return dot; // 1*3 + 2*4 == 11
    }();
    static_assert(zip_ok == 11);

    // --- view: aggregate as a tuple-like via operator| ---
    constexpr auto view = p | csl::ag::views::all;
    static_assert(std::tuple_size_v<decltype(view)> == 2);
    static_assert(std::get<0>(view) == 3);

    // --- opt-in: fmt formatting (CSL_AG__ENABLE_FMTLIB_SUPPORT) ---
#if defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
    fmt::print("{}\n",                    p);  // compact:  (3, 4)
    fmt::print("{}\n", p | csl::ag::io::indented);  // indented: multi-line
#endif

    // --- opt-in: std::format (CSL_AG__ENABLE_FORMAT_SUPPORT) ---
#if defined(CSL_AG__ENABLE_FORMAT_SUPPORT)
    std::print("{}\n",                    p);
    std::print("{}\n", p | csl::ag::io::indented);
#endif
}
