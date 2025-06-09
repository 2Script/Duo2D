#pragma once
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/make.hpp"
#include <result/verify.h>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<typename InputContainerT>
    result<std::pair<buffer, device_memory<1>>> renderable_allocator::stage(std::size_t total_buffer_size, InputContainerT&& inputs, std::size_t buffer_write_offset) const noexcept {
        std::array<buffer, 1> staging_buffs;
        RESULT_TRY_MOVE(staging_buffs[0], make<buffer>(*logi_device_ptr, total_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
        RESULT_TRY_MOVE_UNSCOPED(device_memory<1> staging_mem, 
            make<device_memory<1>>(*logi_device_ptr, *phys_device_ptr, staging_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sm);
        RESULT_VERIFY(staging_mem.bind(*logi_device_ptr, staging_buffs[0], 0));

        //Fill staging buffer with data
        RESULT_TRY_COPY_UNSCOPED(void* mapped_data, staging_mem.map(*logi_device_ptr, total_buffer_size), smm);
        std::byte* data_ptr = static_cast<std::byte*>(mapped_data) + buffer_write_offset; 
        for(std::size_t i = 0; i < std::forward<InputContainerT>(inputs).size(); ++i) {
            std::memcpy(data_ptr, std::forward<InputContainerT>(inputs)[i].data(), std::forward<InputContainerT>(inputs)[i].size_bytes());
            data_ptr += std::forward<InputContainerT>(inputs)[i].size_bytes();
        }
        staging_mem.unmap(*logi_device_ptr);
        return std::pair{std::move(staging_buffs[0]), std::move(staging_mem)};
    }
}


namespace d2d {
    template<std::size_t I, VkFlags BufferUsage, VkMemoryPropertyFlags MemProps, VkMemoryPropertyFlags FallbackMemProps, std::size_t N>
    result<void> renderable_allocator::alloc_buffer(std::span<buffer, N> buffs, std::size_t total_buffer_size, device_memory<N>& mem) noexcept {
        //Create copy command buffer
        RESULT_VERIFY(gpu_alloc_begin());

        //Create the device local buffer
        RESULT_TRY_MOVE(buffs[I], make<buffer>(*logi_device_ptr, total_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | BufferUsage));

        //Re-allocate the memory
        device_memory<N> old_mem = std::move(mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
        auto m = make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, MemProps);
        if(m.has_value()) mem = *std::move(m);
        else if(FallbackMemProps != 0 && m.error() == error::device_lacks_suitable_mem_type) {
            RESULT_TRY_MOVE_UNSCOPED(mem, make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, FallbackMemProps), r)
        }
        else return m.error();
        

        //Re-create all other buffers
        std::array<buffer, N> new_buffs = {};
        RESULT_VERIFY(realloc(new_buffs, buffs, mem, I));

        //Clear copy command buffer
        RESULT_VERIFY(gpu_alloc_end());


        //Replace the old buffer with the new one
        move(buffs, new_buffs);

        return {};
    }
}


namespace d2d {
    result<void> renderable_allocator::staging_to_device_local(buffer& device_local_buff, buffer const& staging_buff, std::size_t offset, std::optional<std::size_t> size) noexcept {
        //Create copy command buffer
        RESULT_VERIFY(gpu_alloc_begin());
        //Copy from staging buffer to device local buffer
        copy_cmd_buffer.copy_alike(device_local_buff, staging_buff, size.value_or(staging_buff.size()), offset);
        //Clear copy command buffer
        RESULT_VERIFY(gpu_alloc_end());
        return {};
    }

    result<void> renderable_allocator::staging_to_device_local(image& device_local_image, buffer const& staging_buff) noexcept {
        std::size_t length = device_local_image.size_bytes();
        if(length != staging_buff.size())
            return errc::invalid_image_initialization;
        
        //Create copy command buffer
        RESULT_VERIFY(gpu_alloc_begin());
        //Copy from staging buffer to device local buffer
        copy_cmd_buffer.transition_image(device_local_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED);
        copy_cmd_buffer.copy_buffer_to_image(device_local_image, staging_buff, device_local_image.size());
        copy_cmd_buffer.transition_image(device_local_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        //Clear copy command buffer
        RESULT_VERIFY(gpu_alloc_end());
        return {};
    }
}



namespace d2d {
    result<void> renderable_allocator::gpu_alloc_begin() noexcept {
        //Create copy command buffer
        RESULT_TRY_MOVE(copy_cmd_buffer, make<command_buffer>(*logi_device_ptr, *copy_cmd_pool_ptr));
        RESULT_VERIFY(copy_cmd_buffer.generic_begin());
        return {};
    }

    result<void> renderable_allocator::gpu_alloc_end() noexcept {
        //Clear copy command buffer
        RESULT_VERIFY(copy_cmd_buffer.generic_end(*logi_device_ptr, *copy_cmd_pool_ptr));
        copy_cmd_buffer = {};
        return {};
    }
}


namespace d2d {
    template<typename OutputContainerT, typename InputContainerT, std::size_t N>
    result<std::size_t> renderable_allocator::realloc(OutputContainerT&& output_container, InputContainerT&& input_container, device_memory<N>& new_mem, std::size_t skip_idx, std::size_t starting_offset) const noexcept {
        //Re-create all other buffers
        std::size_t mem_offset = starting_offset;
        std::size_t i = 0;
        for(auto iter = std::forward<InputContainerT>(input_container).begin(); iter != std::forward<InputContainerT>(input_container).end(); ++iter, ++i) {
            auto& elem = [&iter]() noexcept -> auto& {
                if constexpr(requires {iter->second;}) return iter->second;
                else return (*iter); 
            }();
            if(elem.empty()) continue;
            
            //Create a new buffer with the same properties as the old one
            RESULT_TRY_MOVE(std::forward<OutputContainerT>(output_container)[i], elem.clone(*logi_device_ptr, *phys_device_ptr));
            if constexpr(requires { elem.idx; }) {
                std::forward<OutputContainerT>(output_container)[i].idx = static_cast<texture_idx_t>(i);
            }
            
            //Bind the newly allocated memory to this new buffer
            RESULT_VERIFY(new_mem.bind(*logi_device_ptr, std::forward<OutputContainerT>(output_container)[i], mem_offset));
            mem_offset += new_mem.requirements()[i].size;//(std::forward<OutputContainerT>(output_container)[i].size_bytes() + new_mem.requirements()[i].alignment - 1) & ~(new_mem.requirements()[i].alignment - 1);

            if constexpr(requires (logical_device& l, physical_device& p, VkFormat f, pt3<VkSamplerAddressMode> a){ elem.initialize(l, p, f, a); }) {
                RESULT_VERIFY(std::forward<OutputContainerT>(output_container)[i].initialize(*logi_device_ptr, *phys_device_ptr, elem.format(), elem.sampler().address_modes()));
            }

            //Copy the data from all other old buffers into the new ones
            if(i == skip_idx) continue;
            copy_cmd_buffer.copy_alike(std::forward<OutputContainerT>(output_container)[i], elem, std::forward<OutputContainerT>(output_container)[i].size());
        }

        return {mem_offset};
    }
}


namespace d2d {
    template<typename OutputContainerT, typename InputContainerT>
    void renderable_allocator::move(OutputContainerT&& output_container, InputContainerT&& input_container) const noexcept {
        std::size_t i = 0;
        for(auto iter = std::forward<OutputContainerT>(output_container).begin(); iter != std::forward<OutputContainerT>(output_container).end(); ++iter, ++i) {
            auto& elem = [&iter]() noexcept -> auto& {
                if constexpr(requires {iter->second;}) return iter->second;
                else return (*iter); 
            }();

            if(!elem.empty())
                elem = std::move(std::forward<InputContainerT>(input_container)[i]);
        }
    }
}