#pragma once
#include "Duo2D/vulkan/memory/shader_module.hpp"
#include "Duo2D/core/error.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    template<std::size_t N>
    result<shader_module> shader_module::create(std::shared_ptr<logical_device> device, const unsigned char (&data)[N], VkShaderStageFlagBits type) noexcept {
        shader_module ret{};
        ret.dependent_handle = device;

        //TODO verify alignment of passed shader data OR enforce constexpr
        VkShaderModuleCreateInfo shader_module_create_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = N,
            .pCode = reinterpret_cast<uint32_t const*>(data),
        };
        __D2D_VULKAN_VERIFY(vkCreateShaderModule(*device, &shader_module_create_info, nullptr, &ret.handle));

        ret.shader_stage_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = type,
            .module = ret,
            .pName = "main",
        };

        return ret;
    }
}
