#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::uint32_t FramesInFlight, std::size_t DescriptorCount>
    result<descriptor_pool<FramesInFlight, DescriptorCount>> descriptor_pool<FramesInFlight, DescriptorCount>::create(logical_device& device) noexcept {
        descriptor_pool ret{};
        ret.dependent_handle = device;

        VkDescriptorPoolSize pool_size{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = FramesInFlight * DescriptorCount
        };
        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FramesInFlight * DescriptorCount,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size,
        };
        
        __D2D_VULKAN_VERIFY(vkCreateDescriptorPool(device, &pool_info, nullptr, &ret.handle));
        return ret;
    }
}