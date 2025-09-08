#pragma once
#include "Duo2D/vulkan/memory/device_memory.hpp"

#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include <numeric>
#include <result/verify.h>
#include <span>
#include <utility>
#include <vulkan/vulkan_core.h>


namespace d2d::vk {
    template<std::size_t N>
    result<device_memory<N>> device_memory<N>::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device_weak_ref, std::span<buffer, N> associated_buffers, VkMemoryPropertyFlags properties) noexcept {
        device_memory ret{};    
        ret.dependent_handle = logi_device;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device, phys_device_weak_ref);

        for(std::size_t i = 0; i < N; ++i) {
            if(associated_buffers[i].empty()) ret.mem_reqs[i] = {};
            else vkGetBufferMemoryRequirements(*logi_device, static_cast<VkBuffer>(associated_buffers[i]), &ret.mem_reqs[i]);
        }

        std::optional<std::uint32_t> mem_type_idx = std::nullopt;
        //TODO do this once in physical_device
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device, &mem_props);

        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            for(std::size_t j = 0; j < N; ++j)
                if(!associated_buffers[j].empty() && !(ret.mem_reqs[j].memoryTypeBits & (1 << i)))
                    goto next_mem_prop;
            if ((mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
                mem_type_idx.emplace(i);
                break;
            }
        next_mem_prop:;
        }
        
        RESULT_VERIFY(ret.allocate(logi_device, mem_type_idx, ret.mem_reqs));
        return ret;
    }
    
    template<std::size_t N>
    result<device_memory<N>> device_memory<N>::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device_weak_ref, std::span<image, N> associated_images, VkMemoryPropertyFlags properties) noexcept {
        device_memory ret{};    
        ret.dependent_handle = logi_device;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device, phys_device_weak_ref);

        for(std::size_t i = 0; i < N; ++i) {
            if(associated_images[i].empty()) ret.mem_reqs[i] = {};
            else vkGetImageMemoryRequirements(*logi_device, static_cast<VkImage>(associated_images[i]), &ret.mem_reqs[i]);
        }

        std::optional<std::uint32_t> mem_type_idx = std::nullopt;
        //TODO do this once in physical_device
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device, &mem_props);

        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            for(std::size_t j = 0; j < N; ++j)
                if(!associated_images[j].empty() && !(ret.mem_reqs[j].memoryTypeBits & (1 << i)))
                    goto next_mem_prop;
            if ((mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
                mem_type_idx.emplace(i);
                break;
            }
        next_mem_prop:;
        }
        
        RESULT_VERIFY(ret.allocate(logi_device, mem_type_idx, ret.mem_reqs));
        return ret;
    }


    result<device_memory<std::dynamic_extent>> device_memory<std::dynamic_extent>::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device_weak_ref, texture_map_base& textures, VkMemoryPropertyFlags properties) noexcept {
        device_memory ret{};
        ret.dependent_handle = logi_device;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device, phys_device_weak_ref);

        ret.mem_reqs.reserve(textures.size());

        for(auto iter = textures.cbegin(); iter != textures.cend(); ++iter) {
            if(iter->second.empty()) continue;
            VkMemoryRequirements reqs;
            vkGetImageMemoryRequirements(*logi_device, iter->second, &reqs);
            ret.mem_reqs.push_back(reqs);
        }


        std::optional<std::uint32_t> mem_type_idx = std::nullopt;
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device, &mem_props);

        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            std::size_t j = 0;
            for(auto iter = textures.cbegin(); iter != textures.cend(); ++iter, ++j)
                if(!iter->second.empty() && !(ret.mem_reqs[j].memoryTypeBits & (1 << i)))
                    goto next_mem_prop;
            if ((mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
                mem_type_idx.emplace(i);
                break;
            }
        next_mem_prop:;
        }
        
        RESULT_VERIFY(ret.allocate(logi_device, mem_type_idx, ret.mem_reqs));
        return ret;
    }
}

namespace d2d::vk {
    template<typename MemReqsContainer>
    result<void> device_memory_base::allocate(std::weak_ptr<logical_device> device_weak_ref, std::optional<std::uint32_t> mem_type_idx, MemReqsContainer&& mem_reqs) noexcept {
        __D2D_WEAK_PTR_TRY_LOCK(device, device_weak_ref);
        if(!mem_type_idx.has_value())
            return error::device_lacks_suitable_mem_type;

        VkMemoryAllocateInfo malloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = std::accumulate(std::forward<MemReqsContainer>(mem_reqs).cbegin(), std::forward<MemReqsContainer>(mem_reqs).cend(), static_cast<std::size_t>(0), [](std::size_t sum, VkMemoryRequirements mem_req){ 
                if(mem_req.size == 0) return sum;
                return (mem_req.size + ((sum + mem_req.alignment - 1) & ~(mem_req.alignment - 1))); 
            }),
            .memoryTypeIndex = *mem_type_idx,
        };

        if(malloc_info.allocationSize == 0) return {};
        __D2D_VULKAN_VERIFY(vkAllocateMemory(*device, &malloc_info, nullptr, &handle));
        return {};
    }
}


namespace d2d::vk {
    result<void> device_memory_base::unmap(std::weak_ptr<logical_device> device_weak_ref) const noexcept {
        __D2D_WEAK_PTR_TRY_LOCK(device, device_weak_ref);
        mapped = false;
        vkUnmapMemory(*device, handle);
        return {};
    }

    result<void*> device_memory_base::map(std::weak_ptr<logical_device> device_weak_ref, std::size_t size, std::size_t offset) const noexcept {
        __D2D_WEAK_PTR_TRY_LOCK(device, device_weak_ref);
        if(mapped) RESULT_VERIFY(unmap(device_weak_ref));
        void* map;
        __D2D_VULKAN_VERIFY(vkMapMemory(*device, handle, offset, size, 0, &map));
        mapped = true;
        return map;
    }
}

namespace d2d::vk {
    result<void> device_memory_base::bind(std::weak_ptr<logical_device> device_weak_ref, buffer& buff, std::size_t offset) const noexcept {
        __D2D_WEAK_PTR_TRY_LOCK(device, device_weak_ref);
        __D2D_VULKAN_VERIFY(vkBindBufferMemory(*device, buff, handle, offset));
        buff.offset = offset;
        return {};
    }

    result<void> device_memory_base::bind(std::weak_ptr<logical_device> device_weak_ref, image& img, std::size_t offset) const noexcept {
        __D2D_WEAK_PTR_TRY_LOCK(device, device_weak_ref);
        __D2D_VULKAN_VERIFY(vkBindImageMemory(*device, img, handle, offset));
        img.offset = offset;
        return {};
    }
}

/*
namespace d2d::vk {
    constexpr device_memory_base::device_memory_base(device_memory_base&& other) noexcept : 
        vulkan_ptr<VkDeviceMemory, vkFreeMemory>(std::move(other)),
        mapped(other.mapped) {}

    constexpr device_memory_base& device_memory_base::operator=(device_memory_base&& other) noexcept { 
        vulkan_ptr<VkDeviceMemory, vkFreeMemory>::operator=(std::move(other));
        mapped = other.mapped;
        return *this;
    }


    template<std::size_t N>
    constexpr device_memory<N>::device_memory(device_memory<N>&& other) noexcept : 
        device_memory_base(std::move(other)), 
        mem_reqs(other.mem_reqs) {}

    template<std::size_t N>
    constexpr device_memory<N>& device_memory<N>::operator=(device_memory<N>&& other) noexcept { 
        device_memory_base::operator=(std::move(other));
        mem_reqs = other.mem_reqs;
        return *this;
    }

    template<>
    constexpr device_memory<std::dynamic_extent>::device_memory(device_memory&& other) noexcept : 
        device_memory_base(std::move(other)), 
        mem_reqs(std::move(other.mem_reqs)) {}

    template<>
    constexpr device_memory<std::dynamic_extent>& device_memory<std::dynamic_extent>::operator=(device_memory&& other) noexcept { 
        device_memory_base::operator=(std::move(other));
        mem_reqs = std::move(other.mem_reqs);
        return *this;
    }
}
*/