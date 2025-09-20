#pragma once
#include <string>
#include <unordered_map>
#include "Duo2D/vulkan/display/sampled_image.hpp"

namespace d2d::vk {
    using texture_map_base = std::unordered_map<std::string, sampled_image>;
}