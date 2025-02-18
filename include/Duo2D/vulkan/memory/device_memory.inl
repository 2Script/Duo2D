#pragma once
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include <numeric>
#include <result/verify.h>
#include <utility>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t N>
    result<device_memory<N>> device_memory<N>::create(logical_device& logi_device, physical_device& phys_device, std::span<buffer, N> associated_buffers, VkMemoryPropertyFlags properties) noexcept {
        device_memory ret{};
        ret.dependent_handle = logi_device;

        for(std::size_t i = 0; i < N; ++i) {
            if(associated_buffers[i].empty()) continue;
            switch(associated_buffers[i].type()) {
            case buffer_type::generic:
                vkGetBufferMemoryRequirements(logi_device, static_cast<VkBuffer>(associated_buffers[i]), &ret.mem_reqs[i]);
                break;
            case buffer_type::image:
                vkGetImageMemoryRequirements(logi_device, static_cast<VkImage>(associated_buffers[i]), &ret.mem_reqs[i]);
                break;
            }
        }

        std::optional<std::uint32_t> mem_type_idx = std::nullopt;
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(phys_device, &mem_props);

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

        if(!mem_type_idx.has_value())
            return error::device_lacks_suitable_mem_type;


        VkMemoryAllocateInfo malloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = std::accumulate(ret.mem_reqs.cbegin(), ret.mem_reqs.cend(), static_cast<std::size_t>(0), 
                [](std::size_t sum, VkMemoryRequirements const& mem_req){ return sum + mem_req.size; }),
            .memoryTypeIndex = *mem_type_idx,
        };

        if(malloc_info.allocationSize == 0) return ret;
        __D2D_VULKAN_VERIFY(vkAllocateMemory(logi_device, &malloc_info, nullptr, &ret.handle));
        return ret;
    }
}

namespace d2d {
    template<std::size_t N>
    void device_memory<N>::unmap(logical_device& device) const noexcept {
        mapped = false;
        return vkUnmapMemory(device, handle);
    }

    template<std::size_t N>
    result<void*> device_memory<N>::map(logical_device& device, std::size_t size, std::size_t offset) const noexcept {
        if(mapped) unmap(device);
        void* map;
        __D2D_VULKAN_VERIFY(vkMapMemory(device, handle, offset, size, 0, &map));
        mapped = true;
        return map;
    }


    template<std::size_t N>
    result<void> device_memory<N>::bind(logical_device& device, buffer& buff, std::size_t offset) const noexcept {
        switch(buff.type()){
        case buffer_type::generic:
            __D2D_VULKAN_VERIFY(vkBindBufferMemory(device, static_cast<VkBuffer>(buff), handle, offset));
            break;
        case buffer_type::image:
            __D2D_VULKAN_VERIFY(vkBindImageMemory(device, static_cast<VkImage>(buff), handle, offset));
            break;
        }
        return {};
    }
}


namespace d2d {
    template<std::size_t N>
    constexpr device_memory<N>::device_memory(device_memory&& other) noexcept : 
        vulkan_ptr<VkDeviceMemory, vkFreeMemory>(std::move(other)), 
        mem_reqs(other.mem_reqs),
        mapped(other.mapped) {}

    template<std::size_t N>
    constexpr device_memory<N>& device_memory<N>::operator=(device_memory&& other) noexcept { 
        vulkan_ptr<VkDeviceMemory, vkFreeMemory>::operator=(std::move(other));
        mem_reqs = other.mem_reqs;
        mapped = other.mapped;
        return *this;
    }
}