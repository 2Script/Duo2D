#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

#include "Duo2D/arith/vector.hpp"


namespace d2d {
    template<std::size_t Channels, std::size_t BPC> requires (BPC <= 16 && Channels <= 4)
    struct basic_color {
        using component_type = std::conditional_t<BPC <= 8, std::uint8_t, std::uint16_t>;
        using value_type = 
            std::conditional_t<BPC * Channels <=  8, std::uint8_t,
            std::conditional_t<BPC * Channels <= 16, std::uint16_t,
            std::conditional_t<BPC * Channels <= 32, std::uint32_t, 
        std::uint64_t>>>;

        constexpr static std::size_t component_max = std::numeric_limits<component_type>::max() >> ((sizeof(component_type) * 8) - BPC);
        constexpr static VkFormat format = impl::vertex_traits<Channels, component_type>::format;

    private:
        constexpr static bool perfect_sizing = (Channels * sizeof(component_type)) == sizeof(value_type);

    public:
        constexpr basic_color() noexcept = default;
        constexpr basic_color(std::array<component_type, Channels> color_data) noexcept : data(color_data) {}
        constexpr basic_color(const component_type (&color_arr)[Channels]) noexcept : data(std::to_array(color_arr)) {}

        constexpr basic_color(value_type color_value) noexcept : data{} {
            //can't use memcpy or bitcast beacuse of endian-ness
            for(std::size_t i = 0; i < Channels; ++i)
                data[i] = static_cast<component_type>(color_value >> (BPC * (Channels - 1 - i)));
        }

        //TODO add rgb, hsv, and hsl converting constructors

    public:
        constexpr operator vector<Channels, component_type>() const noexcept { return {data}; }
        constexpr vector<Channels, float> normalize() const noexcept {
            return [this]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                return vector<Channels, float>{(data[I]/static_cast<float>(component_max))...};
            }(std::make_index_sequence<Channels>{});
        }

        //TODO add rgb, hsv, and hsl converting functions

    private:
        std::array<component_type, Channels> data;

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