#pragma once
#include <array>
#include <vulkan/vulkan.h>


namespace d2d {
    namespace feature {
    enum id { 
        geometry_shaders = 4,

        num_features = 55
    };
    }
}


namespace d2d {
    using features_t = std::array<VkBool32, feature::num_features>;
}