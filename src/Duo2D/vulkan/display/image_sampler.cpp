#include "Duo2D/vulkan/display/image_sampler.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"

namespace d2d::vk {
    result<image_sampler> image_sampler::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, pt3<VkSamplerAddressMode> address_modes) noexcept {
        image_sampler ret{};
        ret.dependent_handle = logi_device;
        ret.addr_modes = address_modes;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device_ptr, phys_device);

	    const VkSamplerCreateInfo sampler_create_info {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	        .magFilter               = VK_FILTER_LINEAR,
	        .minFilter               = VK_FILTER_LINEAR,
	        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	        .addressModeU            = address_modes[0],
	        .addressModeV            = address_modes[1],
	        .addressModeW            = address_modes[2],
            .mipLodBias              = 0.0f,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = phys_device_ptr->limits.maxSamplerAnisotropy,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .minLod                  = 0.0f,
	        .maxLod                  = VK_LOD_CLAMP_NONE,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        __D2D_VULKAN_VERIFY(vkCreateSampler(*logi_device, &sampler_create_info, nullptr, &ret.handle));
        return ret;
    }
}