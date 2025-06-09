#pragma once
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include <bit>
#include <limits>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t N>
    result<descriptor_set_layout> descriptor_set_layout::create(logical_device& device, std::span<VkDescriptorSetLayoutBinding, N> bindings, std::span<VkDescriptorBindingFlags, N> binding_flags, std::bitset<N> enabled_bindings) noexcept {
        descriptor_set_layout ret{};
        ret.dependent_handle = device;
        const std::uint32_t binding_count = std::numeric_limits<unsigned long long>::digits - std::countl_zero(enabled_bindings.to_ullong());
        
        VkDescriptorSetLayoutBindingFlagsCreateInfo descriptor_layout_flags_info {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
	        .bindingCount  = binding_count,
	        .pBindingFlags = binding_flags.data(),
        };

        VkDescriptorSetLayoutCreateInfo descriptor_layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &descriptor_layout_flags_info,
            .bindingCount = binding_count,//static_cast<std::uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };
        __D2D_VULKAN_VERIFY(vkCreateDescriptorSetLayout(device, &descriptor_layout_info, nullptr, &ret.handle));
        return ret;
    }
}