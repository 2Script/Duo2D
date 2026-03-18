#pragma once
#include <array>
#include <vulkan/vulkan.h>


namespace acma::vk {
    namespace feature {
    enum id { 
        geometry_shaders = 4,

        num_features = 55
    };
    }
}


namespace acma::vk {
    using features_t = std::array<VkBool32, feature::num_features>;
}