#pragma once
#include "Duo2D/vulkan/display/image_sampler.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/vulkan/memory/image.hpp"

namespace d2d::vk {
    using texture_idx_t = std::uint16_t;

    class texture : public image {
    public:
        static result<texture> create(std::shared_ptr<logical_device> logi_device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, std::uint32_t array_count = 1) noexcept;
        result<void> initialize(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, VkFormat format, pt3<VkSamplerAddressMode> address_modes = {image_sampler::clamp_to_border, image_sampler::clamp_to_border, image_sampler::clamp_to_border}) noexcept;

        result<texture> clone(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device) const noexcept;
    public:
        constexpr image_view    const& view()    const noexcept { return img_view; }
        constexpr image_sampler const& sampler() const noexcept { return img_sampler; }
        constexpr texture_idx_t        index()   const noexcept { return idx; }

    private:
        using image::create;
        using image::clone; 

    private:
        image_view img_view;
        image_sampler img_sampler;
        mutable texture_idx_t idx;
        mutable bool initialized = false;
    public:
        friend class renderable_allocator;
    };
}