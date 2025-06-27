#pragma once
#include <bitset>
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorSetLayout);

namespace d2d::vk {
    struct descriptor_set_layout : vulkan_ptr<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout> {
        template<std::size_t N>
        static result<descriptor_set_layout> create(logical_device& device, std::span<VkDescriptorSetLayoutBinding, N> bindings, std::span<VkDescriptorBindingFlags, N> binding_flags, std::bitset<N> enabled_bindings) noexcept;
    };
}

#include "Duo2D/vulkan/memory/descriptor_set_layout.inl"