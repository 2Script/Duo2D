#include <array>
#include <string_view>
#include <vulkan/vulkan.h>


namespace d2d {
    namespace extension {
    enum id { 
        swap_chain,

        //shader_non_semantic_info,

        num_extensions
    };
    }
}

namespace d2d {
    namespace extension {
        constexpr std::array<std::string_view, extension::num_extensions> name = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            //VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
        }; 
    }
}


namespace d2d {
    using extensions_t = std::array<bool, extension::num_extensions>;
}