#include "Duo2D/vulkan/display/depth_image.hpp"
#include <memory>
#include <span>

#include <result.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"


namespace d2d::vk {
    result<depth_image> depth_image::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, extent2 extent) noexcept {
        depth_image ret{};

        constexpr static VkFormat depth_image_format = VK_FORMAT_D32_SFLOAT;

        RESULT_TRY_MOVE(ret.img, make<image>(logi_device, extent.width(), extent.height(), depth_image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1));

        RESULT_TRY_MOVE(ret.mem, make<device_memory<1>>(logi_device, phys_device, std::span<image, 1>{std::addressof(ret.img), 1}, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        RESULT_VERIFY(ret.mem.bind(logi_device, ret.img, 0));

        RESULT_TRY_MOVE(ret.img_view, make<image_view>(logi_device, ret.img, depth_image_format, 1, VK_IMAGE_ASPECT_DEPTH_BIT));

        return ret;
    }
}
