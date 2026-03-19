#pragma once
#include <memory>

#include <vulkan/vulkan.h>

#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/memory/generic_allocation.fwd.hpp"


namespace acma::vk {
    class depth_image : public vulkan_ptr<VkDeviceMemory, vkFreeMemory> {
    public:
        constexpr depth_image() noexcept = default;
        static result<depth_image> create(std::shared_ptr<logical_device> logi_device, physical_device* phys_device, extent2 extent) noexcept;
    public:
        constexpr auto&& view     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.img_view); }
        constexpr auto&& raw_image(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.img); }

		consteval static VkFormat format() noexcept { return VK_FORMAT_D32_SFLOAT; }
    private:
        image img;
        image_view img_view;
    };
}
