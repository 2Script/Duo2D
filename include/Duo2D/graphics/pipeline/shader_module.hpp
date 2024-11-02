#pragma once
#include <vulkan/vulkan_core.h>
#include <cstddef>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkShaderModule);


namespace d2d {
    struct shader_module : pipeline_obj<VkShaderModule, vkDestroyShaderModule> {
        static result<shader_module> create(logical_device& device, std::string_view name, unsigned char const* data, std::size_t data_size, VkShaderStageFlagBits type) noexcept;
    
    public: 
        inline VkPipelineShaderStageCreateInfo stage_info() const noexcept { return shader_stage_info; }
    private:
        VkPipelineShaderStageCreateInfo shader_stage_info{};
    };
}