#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

#include "Duo2D/arith/vector.hpp"


namespace d2d {
    template<std::size_t Channels, std::size_t BPC>
    using basic_color_array = std::array<std::conditional_t<BPC <= 8, std::uint8_t, std::uint16_t>, Channels>;
}


namespace d2d {
    template<std::size_t Channels, std::size_t BPC> requires (BPC <= 16 && Channels <= 4)
    struct basic_color : public basic_color_array<Channels, BPC> {
        using component_type = typename basic_color_array<Channels, BPC>::value_type;
        using value_type = 
            std::conditional_t<BPC * Channels <=  8, std::uint8_t,
            std::conditional_t<BPC * Channels <= 16, std::uint16_t,
            std::conditional_t<BPC * Channels <= 32, std::uint32_t, 
        std::uint64_t>>>;

        constexpr static std::size_t component_max = std::numeric_limits<component_type>::max() >> ((sizeof(component_type) * 8) - BPC);

    private:
        constexpr static bool perfect_sizing = (Channels * sizeof(component_type)) == sizeof(value_type);

    public:
        constexpr basic_color() noexcept = default;
        constexpr basic_color(std::array<component_type, Channels> color_data) noexcept : basic_color_array<Channels, BPC>{color_data} {}
        constexpr basic_color(const component_type (&color_arr)[Channels]) noexcept : basic_color_array<Channels, BPC>{std::to_array(color_arr)} {}

        constexpr basic_color(value_type color_value) noexcept : basic_color_array<Channels, BPC>{} {
            //can't use memcpy or bitcast beacuse of endian-ness
            for(std::size_t i = 0; i < Channels; ++i)
                (*this)[i] = static_cast<component_type>(color_value >> (BPC * (Channels - 1 - i)));
        }

        //TODO add rgb, hsv, and hsl converting constructors

    public:
        constexpr operator vector<Channels, component_type>() const noexcept { return {*this}; }
        constexpr vector<Channels, float> normalize() const noexcept {
            return [this]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                return vector<Channels, float>{((*this)[I]/static_cast<float>(component_max))...};
            }(std::make_index_sequence<Channels>{});
        }

        //TODO add rgb, hsv, and hsl converting functions

    };
}


namespace d2d {
    using high_color = basic_color<4, 2>;
    using true_color = basic_color<4, 8>;
    using deep_color = basic_color<4, 10>;
    using max_color  = basic_color<4, 16>;
}


namespace d2d::colors {
    //TODO
    //constexpr true_color red = {};
}