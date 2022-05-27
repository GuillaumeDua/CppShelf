#include <csl/srl.hpp>

#include <iostream>
#include <sstream>
#include <cassert>

// temporarly, runtime quick-test are written here
// until #49 is complete

namespace test {
    // Not described aggregate
    struct toto { int i; char c; };
    // Described (defered)
    struct titi {
        int i;
        char c;
        titi * not_serialize_to_avoid_recursivity; // TODO
    };
}
namespace csl::srl {
    template <>
    struct description<test::titi> {
        using type = csl::srl::descriptions::defered_initialization<
            &test::titi::i,
            &test::titi::i,
            &test::titi::c,
            &test::titi::i
        >;
    };
}
namespace test {
    // Described (aggregate)
    struct tutu {

        tutu(int i_arg, char c_arg)
        : i { i_arg }
        , c { c_arg }
        , result { static_cast<std::size_t>(i) + c }
        {}

        decltype(auto) get_i() const noexcept { return i; }
        const auto & get_c() const noexcept { return c; }

    private:
        int i;
        char c;
    public:
        const std::size_t result;
    };
    static_assert(not std::is_standard_layout_v<tutu>);
}
namespace csl::srl {
    template <>
    struct description<test::tutu> {
        using type = csl::srl::descriptions::aggregate_initialization<
            &test::tutu::get_i,
            &test::tutu::get_c
        >;
    };
}
// TODO : Described (constructor) (ex : std::string(std::size_t, CharT) vs std::string{ CharTs... })

auto main() -> int {

    using namespace test;

    std::stringstream ss;
    // Not described - binary/aggregate
    {
        csl::srl::write(ss, toto{ 42, 'A' });
        toto value;
        csl::srl::read_to(ss, value);

        assert(value.i == 42);
        assert(value.c == 'A');
    }
    {
        csl::srl::write(ss, toto{ 42, 'A' });
        auto value = csl::srl::read<toto>(ss);
        assert(value.i == 42);
        assert(value.c == 'A');
    }
    // Defered
    {
        csl::srl::write(ss, titi{ 42, 'A' });
        titi value;
        csl::srl::read_to(ss, value);
        assert(value.i == 42);
        assert(value.c == 'A');
    }
    {
        csl::srl::write(ss, titi{ 42, 'A' });
        auto value = csl::srl::read<titi>(ss);
        assert(value.i == 42);
        assert(value.c == 'A');
    }
    // Described - Aggregate
    {
        csl::srl::write(ss, tutu{42, 'A'});
        tutu value{0,0};
        // csl::srl::read_to(ss, value); // todo

        // assert(value.get_i() == 42);
        // assert(value.get_c() == 'A');
        // assert(value.result == 107);
    }
    {
        csl::srl::write(ss, tutu{42, 'A'});
        auto value = csl::srl::read<tutu>(ss);

        assert(value.get_i() == 42);
        assert(value.get_c() == 'A');
        assert(value.result == 107);
    }
}