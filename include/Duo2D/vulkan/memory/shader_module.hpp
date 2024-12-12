#pragma once
#include <vulkan/vulkan_core.h>
#include <cstddef>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkShaderModule);


namespace d2d {
    struct shader_module : vulkan_ptr<VkShaderModule, vkDestroyShaderModule> {
        template<std::size_t N>
        static result<shader_module> create(logical_device& device, const unsigned char (&data)[N], VkShaderStageFlagBits type) noexcept;

    public: 
        inline VkPipelineShaderStageCreateInfo stage_info() const noexcept { return shader_stage_info; }
    private:
        VkPipelineShaderStageCreateInfo shader_stage_info{};
    };
}

#include "Duo2D/vulkan/memory/shader_module.inl"