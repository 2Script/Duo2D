#pragma once
#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/traits/same_as.hpp"


namespace d2d::impl {
    template<typename T>
    concept asset_like = when_decayed_same_as<T, font> || when_decayed_same_as<T, texture>;
}