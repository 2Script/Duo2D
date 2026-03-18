#pragma once
#include <span>
#include <cstddef>

#include <vulkan/vulkan.h>

#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkShaderModule);


namespace acma::vk {
    struct shader_module : vulkan_ptr<VkShaderModule, vkDestroyShaderModule> {
        template<std::size_t N>
        static result<shader_module> create(std::shared_ptr<logical_device> device, std::array<unsigned char, N> data, VkShaderStageFlagBits type) noexcept;

    public: 
        inline VkPipelineShaderStageCreateInfo stage_info() const noexcept { return shader_stage_info; }
    private:
        VkPipelineShaderStageCreateInfo shader_stage_info{};
    };
}

#include "sirius/vulkan/memory/shader_module.inl"