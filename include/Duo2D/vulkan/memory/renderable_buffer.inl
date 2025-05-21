#pragma once
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/local_renderable.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"
#include <memory>
#include <numeric>
#include <result/verify.h>
#include <type_traits>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    result<renderable_buffer<FiF, Ts...>> renderable_buffer<FiF, Ts...>::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept {
        renderable_buffer ret{};
        ret.logi_device_ptr = std::addressof(logi_device);
        ret.phys_device_ptr = std::addressof(phys_device);
        
        //Create command pool for copy commands
        RESULT_TRY_MOVE(ret.copy_cmd_pool, make<command_pool>(logi_device, phys_device));

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = ((ret.renderable_for<Ts>().static_index_data_bytes.size()) + ...) + ((ret.renderable_for<Ts>().static_vertex_data_bytes.size()) + ...);
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        //Create the compile-time static input data for instanced types
        constexpr static std::size_t static_data_count = ((Ts::instanced ? impl::has_fixed_indices_v<Ts> + impl::has_fixed_vertices_v<Ts> : 0) + ...);
        constexpr static std::array<std::span<const std::byte>, static_data_count> inputs = [](){
            std::array<std::span<const std::byte>, static_data_count> bytes;
            std::size_t idx = 0;
            constexpr auto emplace_instanced_input = []<typename T>(std::array<std::span<const std::byte>, static_data_count>& arr, std::size_t& i){
                if constexpr (T::instanced && impl::has_fixed_indices_v<T>)  arr[i++] = std::span<const std::byte>(local_renderable<T, FiF>::static_index_data_bytes);
                if constexpr (T::instanced && impl::has_fixed_vertices_v<T>) arr[i++] = std::span<const std::byte>(local_renderable<T, FiF>::static_vertex_data_bytes);
            };
            (emplace_instanced_input.template operator()<Ts>(bytes, idx), ...);
            return bytes;
        }();

        //Stage the static input data and allocate it to device-local memory
        RESULT_VERIFY_UNSCOPED(ret.stage(static_buffer_size, inputs), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        RESULT_VERIFY((ret.template alloc_buffer<0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.static_data_buff), 1}, static_buffer_size, ret.static_device_local_mem
        )));
        RESULT_VERIFY((ret.template staging_to_device_local<0>(std::span{std::addressof(ret.static_data_buff), 1}, staging_buffer)));
        }

    create_uniform:
        //If theres no uniform data, just return
        constexpr static std::size_t uniform_buffer_size = (local_renderable<Ts, FiF>::uniform_data_size + ...);
        if constexpr (uniform_buffer_size == 0) return ret;

        //Create uniform buffer
        RESULT_VERIFY((ret.template alloc_buffer<0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.uniform_buff), 1}, uniform_buffer_size, ret.host_mem
        )));

        RESULT_TRY_COPY(ret.uniform_buffer_map, ret.host_mem.map(logi_device, uniform_buffer_size));

        RESULT_TRY_MOVE(ret.desc_pool, (make<descriptor_pool<FiF>>(logi_device, (static_cast<bool>(local_renderable<Ts, FiF>::uniform_data_size) + ...))));

        //Create pipelines and descriptors
        ((ret.renderable_for<Ts>().create_descriptors(logi_device, ret.uniform_buff, ret.desc_pool, static_offsets<Ts>()[buffer_data_type::uniform])), ...);
        ((ret.renderable_for<Ts>().create_pipeline(logi_device, window_render_pass)), ...);

        return ret;
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FiF, Ts...>::apply() noexcept {
        constexpr static std::size_t I = renderable_index<T>;
        constexpr static std::size_t I_a = renderable_index_with_attrib<T>;
        if(renderable_for<T>().data.size() == 0) {
            data_buffs[I] = buffer{};
            return error::invalid_argument;
        }

        //Get the input data bytes, stage them, then move them to device local memory
        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> input_bytes, renderable_for<T>().make_inputs(), _ib);
        RESULT_VERIFY_UNSCOPED(stage(renderable_for<T>().input_size(), input_bytes), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        if(renderable_for<T>().input_size() > data_buffs[I].size()) 
            RESULT_VERIFY((alloc_buffer<I, local_renderable<T, FiF>::usage_flags(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(std::span{data_buffs}, renderable_for<T>().input_size(), device_local_mem)));
        RESULT_VERIFY((staging_to_device_local<I>(std::span{data_buffs}, staging_buffer)));

        
        if constexpr (!impl::has_attributes_v<T>) {
            renderable_for<T>().outdated = false;
            return {};
        }

        if(renderable_for<T>().attribute_buffer_size() == 0) {
            attribute_buffs[I_a] = buffer{};
            return {};
        }
        
        if(renderable_for<T>().attribute_buffer_size() > attribute_buffs[I_a].size()) {
            RESULT_VERIFY((alloc_buffer<I, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT>(
                std::span{attribute_buffs}, renderable_for<T>().attribute_buffer_size(), shared_mem
            )));
        }

        //Must recreate ALL attribute span maps
        RESULT_TRY_COPY_UNSCOPED(void* shared_mem_map, shared_mem.map(*logi_device_ptr, VK_WHOLE_SIZE), smm);
        std::size_t buffer_offset = 0;

        (renderable_for<Ts>().emplace_attributes(buffer_offset, shared_mem_map, shared_mem.requirements()[I_a].alignment), ...);

        renderable_for<T>().outdated = false;
        return {};
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_buffer<FiF, Ts...>::needs_apply() const noexcept {
        return renderable_for<T>().outdated;
    }
}



namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename InputContainerT>
    result<std::pair<buffer, device_memory<1>>> renderable_buffer<FiF, Ts...>::stage(std::size_t total_buffer_size, InputContainerT&& inputs) noexcept {
        std::array<buffer, 1> staging_buffs;
        RESULT_TRY_MOVE(staging_buffs[0], make<buffer>(*logi_device_ptr, total_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
        RESULT_TRY_MOVE_UNSCOPED(device_memory<1> staging_mem, 
            make<device_memory<1>>(*logi_device_ptr, *phys_device_ptr, staging_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sm);
        staging_mem.bind(*logi_device_ptr, staging_buffs[0], 0);

        //Fill staging buffer with data
        RESULT_TRY_COPY_UNSCOPED(void* mapped_data, staging_mem.map(*logi_device_ptr, total_buffer_size), smm);
        std::byte* data_ptr = static_cast<std::byte*>(mapped_data); 
        for(std::size_t i = 0; i < std::forward<InputContainerT>(inputs).size(); ++i) {
            std::memcpy(data_ptr, std::forward<InputContainerT>(inputs)[i].data(), std::forward<InputContainerT>(inputs)[i].size_bytes());
            data_ptr += std::forward<InputContainerT>(inputs)[i].size_bytes();
        }
        staging_mem.unmap(*logi_device_ptr);
        return std::pair{std::move(staging_buffs[0]), std::move(staging_mem)};
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<std::size_t I, VkFlags BufferUsage, VkMemoryPropertyFlags MemProps, VkMemoryPropertyFlags FallbackMemProps, std::size_t N>
    result<void> renderable_buffer<FiF, Ts...>::alloc_buffer(std::span<buffer, N> buffs, std::size_t total_buffer_size, device_memory<N>& mem) noexcept {
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
        
        //Create copy command buffer
        RESULT_TRY_MOVE_UNSCOPED(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        RESULT_VERIFY(copy_cmd_buffer.copy_begin());

        //Re-create all other buffers
        std::array<buffer, N> new_buffs = {};
        std::size_t mem_offset = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if(buffs[i].empty()) continue;
            
            //Create a new buffer with the same properties as the old one
            RESULT_TRY_MOVE(new_buffs[i], buffs[i].clone(*logi_device_ptr));
            
            //Bind the newly allocated memory to this new buffer
            mem.bind(*logi_device_ptr, new_buffs[i], mem_offset);
            mem_offset += (new_buffs[i].size() + mem.requirements()[i].alignment - 1) & ~(mem.requirements()[i].alignment - 1);

            //Copy the data from all other old buffers into the new ones
            if(i == I) continue;
            switch(new_buffs[i].type()){
            case buffer_type::generic:
                copy_cmd_buffer.copy_generic(new_buffs[i], buffs[i], new_buffs[i].size());
                break;
            case buffer_type::image:
                copy_cmd_buffer.copy_image(new_buffs[i], buffs[i], new_buffs[i].image_size());
                break;
            }
        }

        RESULT_VERIFY(copy_cmd_buffer.copy_end(*logi_device_ptr, copy_cmd_pool));

        //Replace the old buffer with the new one
        for(std::size_t i = 0; i < N; ++i)
            if(!buffs[i].empty())
                buffs[i] = std::move(new_buffs[i]);

        return {};
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<std::size_t I, std::size_t N>
    result<void> renderable_buffer<FiF, Ts...>::staging_to_device_local(std::span<buffer, N> device_local_buffs, buffer const& staging_buff) noexcept {
        //Create copy command buffer
        RESULT_TRY_MOVE_UNSCOPED(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        RESULT_VERIFY(copy_cmd_buffer.copy_begin());
        
        //Copy from staging buffer to current device local buffer
        switch(device_local_buffs[I].type()){
        case buffer_type::generic:
            copy_cmd_buffer.copy_generic(device_local_buffs[I], staging_buff, staging_buff.size());
            break;
        case buffer_type::image:
            copy_cmd_buffer.copy_image(device_local_buffs[I], staging_buff, staging_buff.image_size());
            break;
        }


        RESULT_VERIFY(copy_cmd_buffer.copy_end(*logi_device_ptr, copy_cmd_pool));
        return {};
    }
}


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename U> requires impl::RenderableType<std::remove_cvref_t<U>>
    constexpr void renderable_buffer<FiF, Ts...>::push_back(U&& value) noexcept {
        using T = std::remove_cvref_t<U>;
        renderable_for<T>().data.push_back(std::forward<U>(value));
        renderable_for<T>().outdated = true;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T, typename... Args>
    constexpr T& renderable_buffer<FiF, Ts...>::emplace_back(Args&&... args) noexcept {
        T& ret = renderable_for<T>().data.emplace_back(std::forward<Args>(args)...);
        renderable_for<T>().outdated = true;
        return ret;
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::iterator renderable_buffer<FiF, Ts...>::erase(typename std::vector<T>::const_iterator pos) noexcept {
        typename std::vector<T>::iterator ret = renderable_for<T>().data.erase(pos);
        renderable_for<T>().outdated = true;
        return ret;
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::iterator renderable_buffer<FiF, Ts...>::erase(typename std::vector<T>::const_iterator f, typename std::vector<T>::const_iterator l) noexcept {
        typename std::vector<T>::iterator ret = renderable_for<T>().data.erase(f, l);
        renderable_for<T>().outdated = true;
        return ret;
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::const_iterator renderable_buffer<FiF, Ts...>::cbegin() const noexcept {
        return renderable_for<T>().data.cbegin();
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::const_iterator renderable_buffer<FiF, Ts...>::cend() const noexcept {
        return renderable_for<T>().data.cend();
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_buffer<FiF, Ts...>::empty() const noexcept {
        return renderable_for<T>().data.empty();
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::size() const noexcept {
        return renderable_for<T>().data.size();
    }
}


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::index_buffer() const noexcept requires impl::has_indices_v<T> { 
        if constexpr (T::instanced) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::uniform_buffer() const noexcept requires impl::has_uniform_v<T> { 
        return uniform_buff; 
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::vertex_buffer() const noexcept requires impl::has_vertices_v<T>  { 
        if constexpr (T::instanced) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    };

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_buffer<FiF, Ts...>::instance_buffer() const noexcept requires (T::instanced) { 
        return data_buffs[renderable_index<T>]; 
    };

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_buffer<FiF, Ts...>::attribute_buffer() const noexcept requires impl::has_attributes_v<T> { 
        return attribute_buffs[renderable_index_with_attrib<T>]; 
    };


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::index_count(std::size_t i) const noexcept requires (!T::instanced && impl::has_indices_v<T>) {
        return renderable_for<T>().index_count(i);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::vertex_count(std::size_t i) const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return renderable_for<T>().vertex_count(i);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::instance_count() const noexcept { 
        return renderable_for<T>().instance_count();
    }


    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::first_index(std::size_t i) const noexcept requires (!T::instanced && impl::has_indices_v<T>) {
        return renderable_for<T>().first_index(i);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::first_vertex(std::size_t i) const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return renderable_for<T>().first_vertex(i);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::vertex_buffer_offset() const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return renderable_for<T>().vertex_buffer_offset();
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_buffer<FiF, Ts...>::static_offsets() noexcept { 
        //TODO: cleanup
        std::array<buffer_bytes_t, sizeof...(Ts)> buff_offsets;
        constexpr std::array<buffer_bytes_t,sizeof...(Ts)> static_sizes = {
            (buffer_bytes_t{
                local_renderable<Ts, FiF>::static_index_data_bytes.size(),
                local_renderable<Ts, FiF>::uniform_data_size,
                local_renderable<Ts, FiF>::static_index_data_bytes.size(),
                local_renderable<Ts, FiF>::instance_data_size,
                local_renderable<Ts, FiF>::attribute_data_size
            })...
        };
        
        std::exclusive_scan(static_sizes.cbegin(), static_sizes.cend(), buff_offsets.begin(), buffer_bytes_t{},
            [](buffer_bytes_t acc, const buffer_bytes_t& prev_size){ return buffer_bytes_t{
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                acc[buffer_data_type::uniform] + prev_size[buffer_data_type::uniform],
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                0,
                acc[buffer_data_type::attribute] + prev_size[buffer_data_type::attribute],
            };});
        for(std::size_t i = 0; i < sizeof...(Ts); ++i)
            buff_offsets[i][buffer_data_type::vertex] += static_sizes[i][buffer_data_type::index];
        return buff_offsets[renderable_index<T>];
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename T::uniform_type> renderable_buffer<FiF, Ts...>::uniform_map() const noexcept requires impl::has_uniform_v<T> { 
        return {reinterpret_cast<typename T::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + static_offsets<T>()[buffer_data_type::uniform]), FiF}; 
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline<T>& renderable_buffer<FiF, Ts...>::associated_pipeline() const noexcept{ 
        return renderable_for<T>().pl;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline_layout<T>& renderable_buffer<FiF, Ts...>::associated_pipeline_layout() const noexcept{ 
        return renderable_for<T>().pl_layout;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const descriptor_set<FiF>& renderable_buffer<FiF, Ts...>::desc_set() const noexcept{ 
        return renderable_for<T>().set;
    }
}