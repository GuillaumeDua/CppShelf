#pragma once
// old-style std::ostream terminal color support
// you may wanna use fmt/colors.h instead for a more modern feeling https://fmt.dev/latest/api.html#color-api

#include <iosfwd>
#include <iostream>
#include <iomanip>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>

#ifdef _WIN32
# include <Windows.h>
# error "csl::colors: Windows: Not supported yet"
#endif

namespace csl::colors {
    using namespace std::string_view_literals;

    constexpr static std::string_view prefix = "\033[1;";
    constexpr static std::string_view suffix = "m";

    namespace modifier {
        constexpr static std::string_view reset            = "\033[0m";
        constexpr static std::string_view off              = reset;
        constexpr static std::string_view bold             = "\033[1m";
        constexpr static std::string_view bright           = bold;
        constexpr static std::string_view bold_off         = "\033[21m";
        constexpr static std::string_view bright_off       = bold_off;
        constexpr static std::string_view underline        = "\033[4m";
        constexpr static std::string_view underline_off    = "\033[24m";
        constexpr static std::string_view inverse          = "\033[7m";
        constexpr static std::string_view inverse_off      = "\033[27m";
    }

    enum color : char {
        black   = '0',
        red     = '1',
        green   = '2',
        yellow  = '3',
        blue    = '4',
        magenta = '5',
        cyan    = '6',
        white   = '7',
    };

    // API
    template <color> struct foreground{ constexpr static char suffix = '3'; };
    template <color> struct background{ constexpr static char suffix = '4'; };

    template <color value>
    std::ostream & operator<<(std::ostream & os, const foreground<value>){
        return os << prefix << foreground<value>::suffix << value << suffix;
    }
    template <color value>
    std::ostream & operator<<(std::ostream & os, const background<value>){
        return os << prefix << background<value>::suffix << value << suffix;
    }

    // experimental, bad - based on lifetime poor binding
    template <typename, typename> struct scoped_theme;
    template <color fg_value, color bg_value>
    struct scoped_theme<foreground<fg_value>, background<bg_value>>{
        using fg = foreground<fg_value>;
        using bg = background<bg_value>;

        friend std::ostream & operator<<(std::ostream & os, const scoped_theme & value){
            if (value.stream and std::addressof(os) == std::addressof(value.stream.value().get()))
                throw std::runtime_error{ "csl::colors::scoped_theme: already used on another ostream" };
            value.stream = std::ref(os);
            return os << fg{} << bg{};
        }

        scoped_theme() = default;
        scoped_theme(scoped_theme &&) noexcept = default;
        scoped_theme(const scoped_theme &) = default;
        scoped_theme & operator=(scoped_theme &&) noexcept = default;
        scoped_theme & operator=(const scoped_theme &) = default;
        ~scoped_theme(){
            if (not stream)
                return;
            auto & s = stream.value().get();
            s << modifier::reset;
        }
    private:
        using ostream_ref_type = std::reference_wrapper<std::ostream>;
        mutable std::optional<ostream_ref_type> stream;
    };
}

// TODO(Guss): eDSL fg(red) | bg(black) | bold

// TODO(Guss): literals
// "hello"_red, "world"_green