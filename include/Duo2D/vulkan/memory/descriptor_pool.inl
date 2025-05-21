#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::uint32_t FramesInFlight>
    result<descriptor_pool<FramesInFlight>> descriptor_pool<FramesInFlight>::create(logical_device& device, std::uint32_t descriptor_count) noexcept {
        descriptor_pool ret{};
        ret.dependent_handle = device;

        VkDescriptorPoolSize pool_size{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = FramesInFlight * descriptor_count
        };
        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FramesInFlight * descriptor_count,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size,
        };
        
        __D2D_VULKAN_VERIFY(vkCreateDescriptorPool(device, &pool_info, nullptr, &ret.handle));
        return ret;
    }
}