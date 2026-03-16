#include "Duo2D/vulkan/memory/image.hpp"
#include <climits>

namespace d2d::vk {
    result<image> image::create(std::shared_ptr<logical_device> device, VkImageCreateInfo create_info) noexcept {
        image ret{};
		ret.info = create_info;
        ret.dependent_handle = device;
        __D2D_VULKAN_VERIFY(vkCreateImage(*device, &create_info, nullptr, &ret.handle));
		vkGetImageMemoryRequirements(*device, ret.handle, &ret.mem_reqs);

        return ret;
    }
}