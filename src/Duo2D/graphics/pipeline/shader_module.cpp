#include "Duo2D/graphics/pipeline/shader_module.hpp"
#include "Duo2D/error.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<shader_module> shader_module::create(logical_device& device, unsigned char const* data, std::size_t data_size) noexcept {
        shader_module ret{};
        ret.dependent_handle = device;

        //TODO verify alignment of passed shader data OR enforce constexpr
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = data_size;
        shader_module_create_info.pCode = reinterpret_cast<uint32_t const*>(data);
        __D2D_VULKAN_VERIFY(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &ret.handle));

        return ret;
    }
}
