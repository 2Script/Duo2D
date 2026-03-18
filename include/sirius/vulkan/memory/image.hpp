#pragma once
#include <compare>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "sirius/arith/size.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/vulkan/display/pixel_format.hpp"
#include "sirius/core/asset_heap_config.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);

namespace acma::vk {
    struct image : public vulkan_ptr<VkImage, vkDestroyImage> {
        static result<image> create(std::shared_ptr<logical_device> device, VkImageCreateInfo create_info) noexcept;
        
    public:
		constexpr sl::size_t dimensions() const noexcept { return static_cast<sl::size_t>(info.imageType) + 1; } 

		constexpr VkImageCreateInfo     const& creation_info()   const& noexcept { return info; }
        constexpr VkFormat              const& format_id()       const& noexcept { return info.format; }
        constexpr VkExtent3D            const& size()            const& noexcept { return info.extent; }
        constexpr sl::uint32_t          const& mip_level_count() const& noexcept { return info.mipLevels; }
        constexpr sl::uint32_t          const& layer_count()     const& noexcept { return info.arrayLayers; }
        constexpr VkSampleCountFlagBits const& sample_count()    const& noexcept { return info.samples; }
        constexpr VkImageTiling         const& tiling()          const& noexcept { return info.tiling; }
        constexpr VkImageUsageFlags     const& usage()           const& noexcept { return info.usage; }
		
		constexpr VkMemoryRequirements const& memory_requirements() const& noexcept { return mem_reqs; }
        constexpr sl::size_t           const& size_bytes()          const& noexcept { return mem_reqs.size; }
        constexpr sl::size_t           const& alignment()           const& noexcept { return mem_reqs.alignment; }

        constexpr VkImageLayout const& layout() const& noexcept { return this->current_layout; }


    protected:
	    VkImageCreateInfo info;
		VkMemoryRequirements mem_reqs;
		VkImageLayout current_layout;
    public:
		template<sl::index_t, asset_heap_config, typename>
		friend class asset_heap_allocation;
    };
}
