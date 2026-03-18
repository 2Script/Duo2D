#pragma once

#include <vulkan/vulkan.h>

#include "sirius/arith/point.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkSampler);


namespace acma::vk {
    struct image_sampler : public vulkan_ptr<VkSampler, vkDestroySampler> {
        constexpr static VkSamplerAddressMode clamp_to_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    public:
        static result<image_sampler> create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, pt3<VkSamplerAddressMode> address_modes = {clamp_to_border, clamp_to_border, clamp_to_border}) noexcept;
    	static result<image_sampler> create(std::shared_ptr<logical_device> logi_device, VkSamplerCreateInfo create_info) noexcept;

    public:
        constexpr auto&& address_modes(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.addr_modes); }
    private:
        pt3<VkSamplerAddressMode> addr_modes;
    };
}