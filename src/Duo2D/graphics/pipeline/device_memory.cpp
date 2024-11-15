#include "Duo2D/graphics/pipeline/device_memory.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    result<device_memory> device_memory::create(logical_device& logi_device, physical_device& phys_device, buffer& associated_buffer, VkMemoryPropertyFlags properties) noexcept {
        device_memory ret{};
        ret.dependent_handle = logi_device;

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(logi_device, associated_buffer, &mem_reqs);

        std::optional<std::uint32_t> mem_type_idx = std::nullopt;
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(phys_device, &mem_props);

        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((mem_reqs.memoryTypeBits & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
                mem_type_idx.emplace(i);
                break;
            }
        }

        if(!mem_type_idx.has_value())
            return error::device_lacks_suitable_mem_type;


        VkMemoryAllocateInfo malloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_reqs.size,
            .memoryTypeIndex = *mem_type_idx,
        };

        __D2D_VULKAN_VERIFY(vkAllocateMemory(logi_device, &malloc_info, nullptr, &ret.handle));
        vkBindBufferMemory(logi_device, associated_buffer, ret.handle, 0);
        return ret;
    }
}