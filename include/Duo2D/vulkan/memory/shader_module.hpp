#pragma once
#include <span>
#include <cstddef>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkShaderModule);


namespace d2d::vk {
    struct shader_module : vulkan_ptr<VkShaderModule, vkDestroyShaderModule> {
        template<std::size_t N>
        static result<shader_module> create(std::shared_ptr<logical_device> device, std::array<unsigned char, N> data, VkShaderStageFlagBits type) noexcept;

    public: 
        inline VkPipelineShaderStageCreateInfo stage_info() const noexcept { return shader_stage_info; }
    private:
        VkPipelineShaderStageCreateInfo shader_stage_info{};
    };
}

#include "Duo2D/vulkan/memory/shader_module.inl"