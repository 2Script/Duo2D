#pragma once
#include <memory>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/vulkan/memory/image.hpp"
#include "Duo2D/vulkan/memory/device_allocation.fwd.hpp"


namespace d2d::vk {
    class depth_image : public vulkan_ptr<VkDeviceMemory, vkFreeMemory> {
    public:
        constexpr depth_image() noexcept = default;
        static result<depth_image> create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, extent2 extent) noexcept;
    public:
        constexpr auto&& view     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.img_view); }
        constexpr auto&& raw_image(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.img); }

		consteval static VkFormat format() noexcept { return VK_FORMAT_D32_SFLOAT; }
    private:
        image img;
        image_view img_view;
    };
}
