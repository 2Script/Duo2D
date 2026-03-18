#include "sirius/vulkan/display/depth_image.hpp"
#include <memory>
#include <span>

#include <result.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "sirius/core/make.hpp"
#include "sirius/vulkan/memory/device_allocation.hpp"


namespace acma::vk {
    result<depth_image> depth_image::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device_weak_ref, extent2 extent) noexcept {
        depth_image ret{};
		ret.dependent_handle = logi_device;

		const VkImageCreateInfo image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depth_image::format(),
            .extent = {extent.width(), extent.height(), 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        RESULT_TRY_MOVE(ret.img, make<image>(logi_device, image_create_info));

        //TODO do this once in physical_device
        __D2D_WEAK_PTR_TRY_LOCK(phys_device, phys_device_weak_ref);
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device, &mem_props);

		//Find the index of a suitable GPU memory type
		std::uint32_t mem_type_idx = static_cast<std::uint32_t>(sl::npos);
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if(!(ret.img.memory_requirements().memoryTypeBits & (1 << i)))
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
            .allocationSize = ret.img.memory_requirements().size,
            .memoryTypeIndex = mem_type_idx,
        };

        if(malloc_info.allocationSize == 0) return {};
        __D2D_VULKAN_VERIFY(vkAllocateMemory(*logi_device, &malloc_info, nullptr, &ret.handle));
		__D2D_VULKAN_VERIFY(vkBindImageMemory(*logi_device, ret.img, ret.handle, 0));

        RESULT_TRY_MOVE(ret.img_view, make<image_view>(logi_device, ret.img, VK_IMAGE_ASPECT_DEPTH_BIT));

        return ret;
    }
}
