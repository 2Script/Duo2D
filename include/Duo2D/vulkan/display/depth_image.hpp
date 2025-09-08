#pragma once
#include <memory>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/memory/image.hpp"


namespace d2d::vk {
    class depth_image {
    public:
        constexpr depth_image() noexcept = default;
        static result<depth_image> create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, extent2 extent) noexcept;
    public:
        constexpr image_view const& view() const noexcept { return img_view; }
    private:
        image img;
        image_view img_view;
        device_memory<1> mem;
    };
}
