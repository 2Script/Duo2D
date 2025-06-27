#include "Duo2D/vulkan/display/texture.hpp"

#include "Duo2D/core/make.hpp"
#include <memory>
#include <result/verify.h>


namespace d2d::vk {
    result<texture> texture::create(logical_device& logi_device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, std::uint32_t array_count) noexcept {
        texture ret{};
        RESULT_TRY_MOVE(*static_cast<image*>(std::addressof(ret)), make<image>(logi_device, width, height, format, tiling, usage, array_count));
        return ret;
    }

    result<void> texture::initialize(logical_device& logi_device, physical_device& phys_device, VkFormat format, pt3<VkSamplerAddressMode> address_modes) noexcept {
        RESULT_TRY_MOVE(img_view, make<image_view>(logi_device, *this, format, image_count));
        RESULT_TRY_MOVE(img_sampler, make<image_sampler>(logi_device, phys_device, address_modes));
        initialized = true;
        return {};
    }

}

namespace d2d::vk {
    result<texture> texture::clone(logical_device& logi_device, physical_device&) const noexcept {
        return create(logi_device, extent.width(), extent.height(), image_format, image_tiling, flags, image_count);
    }
}