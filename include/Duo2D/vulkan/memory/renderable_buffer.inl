#pragma once
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"
#include <cstring>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/traits/renderable_traits.hpp"


namespace d2d {
    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    result<renderable_buffer<FIF, Ts...>> renderable_buffer<FIF, Ts...>::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept {
        renderable_buffer ret{};
        ret.logi_device_ptr = std::addressof(logi_device);
        ret.phys_device_ptr = std::addressof(phys_device);
        
        //Create command pool for copy commands
        __D2D_TRY_MAKE(ret.copy_cmd_pool, make<command_pool>(logi_device, phys_device), ccp);

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = instanced_buffer_size.index + instanced_buffer_size.vertex;
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        //Create staging buffer for static instance data buffer
        __D2D_TRY_MAKE(buffer static_staging_buff, make<buffer>(logi_device, static_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), isb);
        __D2D_TRY_MAKE(device_memory static_staging_mem, 
            make<device_memory>(logi_device, phys_device, static_staging_buff, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), ism);

        //Fill staging buffer with static instance data
        void* mapped_static_data;
        vkMapMemory(logi_device, static_staging_mem, 0, static_buffer_size, 0, &mapped_static_data);
        ((copy_staging<Ts>(mapped_static_data)), ...);
        vkUnmapMemory(logi_device, static_staging_mem);

        //Create static instance buffer
        __D2D_TRY_MAKE(ret.static_data_buff, make<buffer>(logi_device, static_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), ib);
        __D2D_TRY_MAKE(ret.static_data_mem, make<device_memory>(logi_device, phys_device, ret.static_data_buff, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), im);

        //Copy from index staging buffer to index device buffer
        if(auto c = ret.copy(ret.static_data_buff, static_staging_buff, static_buffer_size); !c.has_value())
            return c.error();
        }

    create_uniform:
        //If theres no uniform data, just return
        constexpr static std::size_t uniform_buffer_size = (data_size<Ts>.uniform + ...) * FIF;
        if constexpr (uniform_buffer_size == 0) return ret;

        //Create uniform buffer
        __D2D_TRY_MAKE(ret.uniform_buff, make<buffer>(logi_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), ub);
        __D2D_TRY_MAKE(ret.uniform_mem, 
            make<device_memory>(logi_device, phys_device, ret.uniform_buff, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), um);
        vkMapMemory(logi_device, ret.uniform_mem, 0, uniform_buffer_size, 0, &ret.uniform_buffer_map);


        __D2D_TRY_MAKE(ret.desc_pool, (make<descriptor_pool<FIF, uniformed_count>>(logi_device)), dp);

        //Create pipelines and descriptors
        ((ret.create_pipelines<Ts>(logi_device, window_render_pass)), ...);

        return ret;
    }

    
    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FIF, Ts...>::create_pipelines(logical_device& logi_device, render_pass& window_render_pass) noexcept {
        if constexpr (impl::UniformRenderableType<T>) {
            constexpr static std::size_t I = uniformed_idx<T>;
            //Create descriptor set layout
            __D2D_TRY_MAKE(desc_set_layouts[I], make<descriptor_set_layout>(logi_device), dsl);

            //Create pipeline layouts with descriptors
            __D2D_TRY_MAKE(pipeline_layouts[idx<T>], make<pipeline_layout>(logi_device, desc_set_layouts[I]), pld);

            //Create descriptor set and pool
            __D2D_TRY_MAKE(desc_sets[I], (make<descriptor_set<FIF>>(logi_device, desc_pool, desc_set_layouts[I], uniform_buff, data_size<T>.uniform, offsets<T>().uniform)), ds);
        } else {
            //Create pipeline layouts
            __D2D_TRY_MAKE(pipeline_layouts[idx<T>], make<pipeline_layout>(logi_device), pl);
        }

        //Create pipelines
        __D2D_TRY_MAKE(std::get<pipeline<T>>(pipelines), make<pipeline<T>>(logi_device, window_render_pass, pipeline_layouts[idx<T>]), p);
        return result<void>{std::in_place_type<void>};
    }
}


namespace d2d {
    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FIF, Ts...>::apply(bool shrink) noexcept {
        if constexpr (!T::instanced && impl::IndexRenderableType<T>) 
            if(auto c = create_buffer<T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT>(shrink, data_size<T>.index, index_buffs[indexed_idx<T>], index_mems[indexed_idx<T>]); !c.has_value())
                return c.error();

        if constexpr (!T::instanced && impl::VertexRenderableType<T>) 
            if(auto c = create_buffer<T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT>(shrink, data_size<T>.vertex, instance_buffs[idx<T>], instance_mems[idx<T>]); !c.has_value())
                return c.error();

        if constexpr (T::instanced) 
            if(auto c = create_buffer<T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT>(shrink, data_size<T>.instance, instance_buffs[idx<T>], instance_mems[idx<T>]); !c.has_value())
                return c.error();
        
        return result<void>{std::in_place_type<void>};
    }


    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T, VkBufferUsageFlags BufferFlag>
    result<void> renderable_buffer<FIF, Ts...>::create_buffer(bool shrink, std::size_t size, buffer& buff, device_memory& mem) noexcept {
        const std::size_t buffer_data_size = size * std::get<std::vector<T>>(*this).size();
        if(buffer_data_size == 0) {
            buff = buffer{};
            mem = device_memory{};
            return result<void>{std::in_place_type<void>};
        }
        //Create staging buffer
        __D2D_TRY_MAKE(buffer staging_buff, make<buffer>(*logi_device_ptr, buffer_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), sb);
        __D2D_TRY_MAKE(device_memory staging_mem, 
            make<device_memory>(*logi_device_ptr, *phys_device_ptr, staging_buff, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sm);

        //Fill staging buffer with data
        auto get_data = [this](std::size_t i) noexcept -> decltype(auto) {
            if constexpr (!T::instanced && (BufferFlag & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)) return std::get<std::vector<T>>(*this)[i].indices();
            else if constexpr (!T::instanced && (BufferFlag & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) return std::get<std::vector<T>>(*this)[i].vertices();
            else return std::get<std::vector<T>>(*this)[i].instance();
        };

        void* mapped_data;
        vkMapMemory(*logi_device_ptr, staging_mem, 0, buffer_data_size, 0, &mapped_data);
        for(std::size_t i = 0; i < std::get<std::vector<T>>(*this).size(); ++i) {
            const auto input = get_data(i);
            if constexpr(!T::instanced)
                std::memcpy(static_cast<char*>(mapped_data) + size * i, input.data(), size);
            else 
                std::memcpy(static_cast<char*>(mapped_data) + size * i, &input, size);
        }
        vkUnmapMemory(*logi_device_ptr, staging_mem);

        //Allocate buffer if it's the wrong size
        if(buffer_data_size > buff.size() || shrink) {
            __D2D_TRY_MAKE(buff, make<buffer>(*logi_device_ptr, buffer_data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | BufferFlag), b);
            __D2D_TRY_MAKE(mem, make<device_memory>(*logi_device_ptr, *phys_device_ptr, buff, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), m);
        }

        //Copy from staging buffer to device buffer
        if(auto c = copy(buff, staging_buff, buffer_data_size); !c.has_value())
            return c.error();
        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    result<void> renderable_buffer<FIF, Ts...>::copy(buffer& dst, const buffer& src, std::size_t size) const noexcept {
        __D2D_TRY_MAKE(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        copy_cmd_buffer.copy(dst, src, size);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &copy_cmd_buffer;
        vkQueueSubmit(logi_device_ptr->queues[queue_family::graphics], 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(logi_device_ptr->queues[queue_family::graphics]);

        vkFreeCommandBuffers(*logi_device_ptr, copy_cmd_pool, 1, &copy_cmd_buffer);
        return result<void>{std::in_place_type<void>};
    }


    template<std::size_t FIF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    void renderable_buffer<FIF, Ts...>::copy_staging(void* data_map) noexcept {
        if constexpr (T::instanced && impl::IndexRenderableType<T>)
            std::memcpy(static_cast<char*>(data_map) + buff_offsets[idx<T>].index, T::indices().data(), data_size<T>.index);
        if constexpr (T::instanced && impl::VertexRenderableType<T>)
            std::memcpy(static_cast<char*>(data_map) + buff_offsets[idx<T>].vertex, T::vertices().data(), data_size<T>.vertex);
    }
}


namespace d2d {

}