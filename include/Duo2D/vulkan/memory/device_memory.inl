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
    result<device_memory<N>> device_memory<N>::create(logical_device& logi_device, physical_device& phys_device, const std::array<buffer, N>& associated_buffers, VkMemoryPropertyFlags properties) noexcept {
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
    result<void> device_memory<N>::bind(logical_device& device, std::array<buffer, N>& associated_buffers, std::size_t buff_idx, command_pool& cmd_pool) const noexcept {
        RESULT_TRY_MOVE_UNSCOPED(command_buffer copy_cmd_buffer, (make<command_buffer>(device, cmd_pool)), ccb);
        if(auto b = copy_cmd_buffer.copy_begin(); !b.has_value()) return b.error();

        std::array<buffer, N> new_buffs = {};
        std::size_t mem_offset = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if(associated_buffers[i].empty()) continue;
            RESULT_TRY_MOVE(new_buffs[i], associated_buffers[i].clone(device));
            switch(associated_buffers[i].type()){
            case buffer_type::generic:
                vkBindBufferMemory(device, static_cast<VkBuffer>(new_buffs[i]), handle, mem_offset);
                if(i != buff_idx) copy_cmd_buffer.copy_generic(new_buffs[i], associated_buffers[i], new_buffs[i].size());
                break;
            case buffer_type::image:
                vkBindImageMemory(device, static_cast<VkImage>(new_buffs[i]), handle, mem_offset);
                if(i != buff_idx) copy_cmd_buffer.copy_image(new_buffs[i], associated_buffers[i], new_buffs[i].image_size());
                break;
            }
            mem_offset += (new_buffs[i].size() + mem_reqs[i].alignment - 1) & ~(mem_reqs[i].alignment - 1);
        }
        
        if(auto e = copy_cmd_buffer.copy_end(device, cmd_pool); !e.has_value()) return e.error();
        associated_buffers = std::move(new_buffs); //std::swap(associated_buffers, new_buffs);
        return {};
    }
}