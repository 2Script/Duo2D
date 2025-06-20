#include "Duo2D/graphics/core/texture.hpp"

#include "Duo2D/vulkan/make.hpp"
#include <memory>
#include <result/verify.h>


namespace d2d {
    result<texture> texture::create(logical_device& logi_device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) noexcept {
        texture ret{};
        RESULT_TRY_MOVE(*static_cast<image*>(std::addressof(ret)), make<image>(logi_device, width, height, format, tiling, usage));
        return std::move(ret);
    }

    result<void> texture::initialize(logical_device& logi_device, physical_device& phys_device, VkFormat format, pt3<VkSamplerAddressMode> address_modes) noexcept {
        RESULT_TRY_MOVE(img_view, make<image_view>(logi_device, *this, format));
        RESULT_TRY_MOVE(img_sampler, make<image_sampler>(logi_device, phys_device, address_modes));
        initialized = true;
        return {};
    }

}

namespace d2d {
    result<texture> texture::clone(logical_device& logi_device, physical_device&) const noexcept {
        return create(logi_device, extent.width(), extent.height(), image_format, image_tiling, flags);
    }
}