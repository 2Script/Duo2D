#include "Duo2D/vulkan/display/depth_image.hpp"
#include <memory>
#include <span>

#include <result.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/memory/device_allocation.hpp"


namespace d2d::vk {
    result<depth_image> depth_image::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device_weak_ref, extent2 extent) noexcept {
        depth_image ret{};
		ret.dependent_handle = logi_device;

        RESULT_TRY_MOVE(ret.img, make<image>(logi_device, extent.width(), extent.height(), depth_image::format(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1));

		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(*logi_device, static_cast<VkImage>(ret.img), &mem_req);

        //TODO do this once in physical_device
        __D2D_WEAK_PTR_TRY_LOCK(phys_device, phys_device_weak_ref);
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device, &mem_props);

		//Find the index of a suitable GPU memory type
		std::uint32_t mem_type_idx = static_cast<std::uint32_t>(sl::npos);
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if(!(mem_req.memoryTypeBits & (1 << i)))
               continue;
            if ((mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                mem_type_idx = i;
                break;
            }
        }
		if(mem_type_idx == static_cast<std::uint32_t>(sl::npos))
            return errc::device_lacks_suitable_mem_type;

		//Allocate the raw memory
        VkMemoryAllocateInfo malloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_req.size,
            .memoryTypeIndex = mem_type_idx,
        };

        if(malloc_info.allocationSize == 0) return {};
        __D2D_VULKAN_VERIFY(vkAllocateMemory(*logi_device, &malloc_info, nullptr, &ret.handle));
		__D2D_VULKAN_VERIFY(vkBindImageMemory(*logi_device, ret.img, ret.handle, 0));

        RESULT_TRY_MOVE(ret.img_view, make<image_view>(logi_device, ret.img, depth_image::format(), 1, VK_IMAGE_ASPECT_DEPTH_BIT));

        return ret;
    }
}
