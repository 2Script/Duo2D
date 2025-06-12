#pragma once
#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include <memory>
#include <numeric>
#include <result/verify.h>
#include <string_view>
#include <type_traits>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    result<renderable_tuple<FiF, Ts...>> renderable_tuple<FiF, Ts...>::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept {
        renderable_tuple ret{};
        ret.logi_device_ptr = std::addressof(logi_device);
        ret.phys_device_ptr = std::addressof(phys_device);
        
        //Create command pool for copy commands
        RESULT_TRY_MOVE(ret.copy_cmd_pool, make<command_pool>(logi_device, phys_device));

        //Create allocator
        renderable_allocator allocator(logi_device, phys_device, ret.copy_cmd_pool);

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = ((ret.renderable_data_of<Ts>().static_index_data_bytes.size()) + ...) + ((ret.renderable_data_of<Ts>().static_vertex_data_bytes.size()) + ...);
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        //Create the compile-time static input data for instanced types
        constexpr static std::size_t static_data_count = ((renderable_constraints<Ts>::instanced ? renderable_constraints<Ts>::has_fixed_indices + renderable_constraints<Ts>::has_fixed_vertices : 0) + ...);
        constexpr static std::array<std::span<const std::byte>, static_data_count> inputs = [](){
            std::array<std::span<const std::byte>, static_data_count> bytes;
            std::size_t idx = 0;
            constexpr auto emplace_instanced_input = []<typename T>(std::array<std::span<const std::byte>, static_data_count>& arr, std::size_t& i){
                if constexpr (renderable_constraints<T>::instanced && renderable_constraints<T>::has_fixed_indices)  arr[i++] = std::span<const std::byte>(renderable_data<T, FiF>::static_index_data_bytes);
                if constexpr (renderable_constraints<T>::instanced && renderable_constraints<T>::has_fixed_vertices) arr[i++] = std::span<const std::byte>(renderable_data<T, FiF>::static_vertex_data_bytes);
            };
            (emplace_instanced_input.template operator()<Ts>(bytes, idx), ...);
            return bytes;
        }();

        //Stage the static input data and allocate it to device-local memory
        RESULT_VERIFY_UNSCOPED(allocator.stage(static_buffer_size, inputs), s);
        auto [staging_buffer, staging_mem] = *std::move(s);

        RESULT_VERIFY((allocator.template alloc_buffer<0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.static_data_buff), 1}, static_buffer_size, ret.static_device_local_mem
        )));
        RESULT_VERIFY(allocator.staging_to_device_local(ret.static_data_buff, staging_buffer));
        }

    create_uniform:
        //If theres no uniform data, skip creating its buffer
        constexpr static std::size_t uniform_buffer_size = (renderable_data<Ts, FiF>::uniform_data_size + ...);
        if constexpr (uniform_buffer_size == 0) goto create_descriptors;

        //Create uniform buffer
        RESULT_VERIFY((allocator.template alloc_buffer<0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.uniform_buff), 1}, uniform_buffer_size, ret.host_mem
        )));

        RESULT_TRY_COPY(ret.uniform_buffer_map, ret.host_mem.map(logi_device, uniform_buffer_size));

    create_descriptors:
        //Create pipelines and descriptors
        //TODO: Simplify this
        errc error_code = error::unknown;
        auto create_descriptors = [&]<typename T>(errc& current_error_code) noexcept -> errc {
            if(current_error_code != error::unknown) return current_error_code;
            RESULT_VERIFY(ret.renderable_data_of<T>().create_uniform_descriptors(ret.uniform_buff, static_offsets<T>()[buffer_data_type::uniform]));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_texture_descriptors(ret.textures, ret.texture_size_buffer));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_pipeline_layout(logi_device));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_pipeline(logi_device, window_render_pass));
            return error::unknown;
        };
        ((error_code = create_descriptors.template operator()<Ts>(error_code)), ...);
        if(error_code != error::unknown) return error_code;

        return ret;
    }
}

namespace d2d {
    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_tuple<FiF, Ts...>::apply(render_pass& window_render_pass) noexcept {
        constexpr static std::size_t I = renderable_index<T>;
        if(renderable_data_of<T>().input_renderables.size() == 0) {
            data_buffs[I] = buffer{};
            return {};
        }
        
        renderable_allocator allocator(*logi_device_ptr, *phys_device_ptr, copy_cmd_pool);
        auto load_texture = [this](std::string_view path) noexcept -> auto {
            return textures.load(path, *logi_device_ptr, *phys_device_ptr, copy_cmd_pool, texture_mem, texture_size_buffer);
        };
        
        //Get the input data bytes, stage them, then move them to device local memory
        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> input_bytes, renderable_data_of<T>().make_inputs(load_texture), ib);
        RESULT_VERIFY_UNSCOPED(allocator.stage(renderable_data_of<T>().input_size(), input_bytes), s);
        renderable_data_of<T>().clear_inputs();
        auto [staging_buffer, staging_mem] = *std::move(s);

        if(renderable_data_of<T>().input_size() > data_buffs[I].size()) 
            RESULT_VERIFY((allocator.template alloc_buffer<I, renderable_data<T, FiF>::usage_flags(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(std::span{data_buffs}, renderable_data_of<T>().input_size(), device_local_mem)));
        RESULT_VERIFY((allocator.staging_to_device_local(data_buffs[I], staging_buffer)));

        if constexpr (renderable_constraints<T>::has_textures) {
            //TODO: Simplify this
            errc error_code = error::unknown;
            auto update_indicies = [&]<typename U>(errc& current_error_code) noexcept -> errc {
                if constexpr(!renderable_constraints<U>::has_textures) return current_error_code;
                else {
                    if(current_error_code != error::unknown) return current_error_code;
                    RESULT_VERIFY(renderable_data_of<U>().template update_texture_indices<T>(load_texture, allocator, data_buffs[renderable_index<U>]));
                    RESULT_VERIFY(renderable_data_of<U>().create_texture_descriptors(textures, texture_size_buffer));
                    RESULT_VERIFY(renderable_data_of<U>().create_pipeline_layout(*logi_device_ptr));
                    RESULT_VERIFY(renderable_data_of<U>().create_pipeline(*logi_device_ptr, window_render_pass));
                    return error::unknown;
                }
            };
            ((error_code = update_indicies.template operator()<Ts>(error_code)), ...);
            (renderable_data_of<Ts>().clear_inputs(), ...);
            if(error_code != error::unknown) return error_code;
        }
        
        if constexpr (!renderable_constraints<T>::has_attributes) {
            renderable_data_of<T>().outdated = false;
            return {};
        }

        constexpr static std::size_t I_a = renderable_index_with_attrib<T>;
        (renderable_data_of<Ts>().unbind_attributes(), ...);

        if(renderable_data_of<T>().attribute_buffer_size() == 0) {
            attribute_buffs[I_a] = buffer{};
            return {};
        }
        
        if(renderable_data_of<T>().attribute_buffer_size() > attribute_buffs[I_a].size()) {
            RESULT_VERIFY((allocator.template alloc_buffer<I_a, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT>(
                std::span{attribute_buffs}, renderable_data_of<T>().attribute_buffer_size(), shared_mem
            )));
        }

        //Must recreate ALL attribute span maps
        RESULT_TRY_COPY_UNSCOPED(void* shared_mem_map, shared_mem.map(*logi_device_ptr, VK_WHOLE_SIZE), smm);
        std::size_t buffer_offset = 0;

        (renderable_data_of<Ts>().emplace_attributes(buffer_offset, shared_mem_map, shared_mem.requirements()[renderable_index_with_attrib<Ts>].size), ...);

        renderable_data_of<T>().outdated = false;
        return {};
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_tuple<FiF, Ts...>::needs_apply() const noexcept {
        return renderable_data_of<T>().outdated;
    }
}



namespace d2d {
    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_tuple<FiF, Ts...>::empty() const noexcept {
        return renderable_data_of<T>().input_renderables.empty();
    }
}


namespace d2d {
    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, Ts...>::index_buffer() const noexcept requires renderable_constraints<T>::has_indices { 
        if constexpr (renderable_constraints<T>::instanced) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, Ts...>::uniform_buffer() const noexcept requires renderable_constraints<T>::has_uniform { 
        return uniform_buff; 
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, Ts...>::vertex_buffer() const noexcept requires renderable_constraints<T>::has_vertices  { 
        if constexpr (renderable_constraints<T>::instanced) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    };

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, Ts...>::instance_buffer() const noexcept requires (renderable_constraints<T>::instanced) { 
        return data_buffs[renderable_index<T>]; 
    };

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, Ts...>::attribute_buffer() const noexcept requires renderable_constraints<T>::has_attributes { 
        return attribute_buffs[renderable_index_with_attrib<T>]; 
    };

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, Ts...>::texture_idx_buffer() const noexcept requires renderable_constraints<T>::has_textures { 
        return data_buffs[renderable_index<T>];
    };


    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, Ts...>::index_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices) {
        return renderable_data_of<T>().index_count(i);
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, Ts...>::vertex_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_vertices) {
        return renderable_data_of<T>().vertex_count(i);
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, Ts...>::instance_count() const noexcept { 
        return renderable_data_of<T>().instance_count();
    }


    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, Ts...>::first_index(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices) {
        return renderable_data_of<T>().first_index(i);
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, Ts...>::first_vertex(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_vertices) {
        return renderable_data_of<T>().first_vertex(i);
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, Ts...>::vertex_buffer_offset() const noexcept requires (renderable_constraints<T>::has_vertices) {
        if constexpr (renderable_constraints<T>::instanced) return static_offsets<T>()[buffer_data_type::vertex];
        else return renderable_data_of<T>().vertex_buffer_offset();
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, Ts...>::index_buffer_offset() const noexcept requires (renderable_constraints<T>::has_indices) {
        if constexpr (renderable_constraints<T>::instanced) return static_offsets<T>()[buffer_data_type::index];
        else return renderable_data_of<T>().index_buffer_offset();
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, Ts...>::texture_idx_buffer_offset() const noexcept requires (renderable_constraints<T>::has_textures) {
        return renderable_data_of<T>().texture_idx_buffer_offset();
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_tuple<FiF, Ts...>::static_offsets() noexcept { 
        //TODO: cleanup
        std::array<buffer_bytes_t, sizeof...(Ts)> buff_offsets;
        constexpr std::array<buffer_bytes_t,sizeof...(Ts)> static_sizes = {
            (buffer_bytes_t{
                renderable_data<Ts, FiF>::static_index_data_bytes.size(),
                renderable_data<Ts, FiF>::uniform_data_size,
                renderable_data<Ts, FiF>::static_vertex_data_bytes.size(),
                renderable_data<Ts, FiF>::instance_data_size,
                renderable_data<Ts, FiF>::attribute_data_size
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
    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename T::uniform_type> renderable_tuple<FiF, Ts...>::uniform_map() const noexcept requires renderable_constraints<T>::has_uniform { 
        return {reinterpret_cast<typename T::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + static_offsets<T>()[buffer_data_type::uniform]), FiF}; 
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline<T>& renderable_tuple<FiF, Ts...>::associated_pipeline() const noexcept{ 
        return renderable_data_of<T>().pl;
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline_layout<T>& renderable_tuple<FiF, Ts...>::associated_pipeline_layout() const noexcept{ 
        return renderable_data_of<T>().pl_layout;
    }

    template<std::size_t FiF, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const std::array<VkDescriptorSet, FiF>& renderable_tuple<FiF, Ts...>::desc_set() const noexcept{ 
        return renderable_data_of<T>().sets;
    }
}