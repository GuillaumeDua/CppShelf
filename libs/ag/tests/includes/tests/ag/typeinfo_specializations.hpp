#pragma once
// Must be included after <csl/ag.hpp> with a format macro defined.
#include <array>
#include <string_view>
#include <tuple>

namespace csl::ag::io::details {

    template <>
    struct type_name<std::string_view> {
        static constexpr std::string_view value = "std::string_view";
    };

    template <typename... Ts>
    struct type_name<std::tuple<Ts...>> {
    private:
        
        static constexpr std::string_view sep = ", ";
        static constexpr std::size_t storage_size =
            std::string_view{"std::tuple<"}.size()
            + (type_name_v<Ts>.size() + ... + 0)
            + (sizeof...(Ts) > 0 ? (sizeof...(Ts) - 1) * sep.size() : 0)
            + std::string_view{">"}.size();

        static constexpr auto make() noexcept -> std::array<char, storage_size> {

            std::array<char, storage_size> buf{};
            std::size_t pos = 0;
            auto write = [&](std::string_view sv) noexcept {
                for (char c : sv) buf[pos++] = c;
            };
            write("std::tuple<");
            bool first = true;
            ([&] {
                if (not first) write(sep);
                first = false;
                write(type_name_v<Ts>);
            }(), ...);
            write(">");
            return buf;
        }

        static constexpr auto storage = make();

    public:
        static constexpr std::string_view value{storage.data(), storage_size};
    };

    template <typename T, std::size_t N>
    struct type_name<std::array<T, N>> {
    private:

        static constexpr auto make_num_storage() noexcept {
            
            if constexpr (N == 0) {
                return std::array<char, 1>{'0'};
            }
            else {
                constexpr auto len = [] {
                    std::size_t v = N, n = 0;
                    while (v > 0) { v /= 10; ++n; }
                    return n;
                }();
                std::array<char, len> buf{};
                auto v = N;
                auto i = len;
                while (v > 0) { buf[--i] = char('0' + v % 10); v /= 10; }
                return buf;
            }
        }

        static constexpr auto num_storage = make_num_storage();
        static constexpr std::string_view num_sv{num_storage.data(), num_storage.size()};
        static constexpr std::string_view elem_sv = type_name_v<T>;

        static constexpr std::size_t storage_size =
            std::string_view{"std::array<"}.size()
            + elem_sv.size()
            + std::string_view{", "}.size()
            + num_sv.size()
            + std::string_view{">"}.size();

        static constexpr auto make() noexcept -> std::array<char, storage_size> {
            std::array<char, storage_size> buf{};
            std::size_t pos = 0;
            auto write = [&](std::string_view sv) noexcept {
                for (char c : sv)
                    buf[pos++] = c;
            };
            write("std::array<");
            write(elem_sv);
            write(", ");
            write(num_sv);
            write(">");
            return buf;
        }

        static constexpr auto storage = make();

    public:
        static constexpr std::string_view value{storage.data(), storage_size};
    };
}
