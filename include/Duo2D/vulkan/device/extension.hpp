#include <array>
#include <string_view>
#include <vulkan/vulkan.h>


namespace d2d::vk {
    namespace extension {
    enum id { 
        swap_chain,
		push_descriptor,
		//maintenance_5,
		//descriptor_heap,

        num_extensions
    };
    }
}

namespace d2d::vk {
    namespace extension {
        constexpr std::array<std::string_view, extension::num_extensions> name = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
			//VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
            //VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
        }; 
    }
}


namespace d2d::vk {
    using extensions_t = std::array<bool, extension::num_extensions>;
}