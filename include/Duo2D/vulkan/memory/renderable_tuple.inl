#pragma once
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/traits/buffer_traits.hpp"
#include "Duo2D/traits/renderable_properties.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include <memory>
#include <numeric>
#include <result/verify.h>
#include <string_view>
#include <type_traits>
#include <vulkan/vulkan_core.h>


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    result<renderable_tuple<FiF, std::tuple<Ts...>>> renderable_tuple<FiF, std::tuple<Ts...>>::create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_data_map, render_pass& window_render_pass) noexcept {
        renderable_tuple ret{};
        ret.logi_device_ptr = logi_device;
        ret.phys_device_ptr = phys_device;
        ret.font_data_map_ptr = font_data_map;
        
        //Create command pool for copy commands
        vk::command_pool c;
        RESULT_TRY_MOVE(c, make<command_pool>(logi_device, phys_device));
        ret.copy_cmd_pool_ptr = std::make_shared<command_pool>(std::move(c));

        //Create allocator
        renderable_allocator allocator(logi_device, phys_device, ret.copy_cmd_pool_ptr);

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
        goto create_uniform;
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
        goto create_descriptors;

    create_descriptors:
        //Create pipelines and descriptors
        //TODO: Simplify this
        errc error_code = error::unknown;
        auto create_descriptors = [&]<typename T>(errc& current_error_code) noexcept -> errc {
            if(current_error_code != error::unknown) return current_error_code;
            RESULT_VERIFY(ret.renderable_data_of<T>().create_uniform_descriptors(ret.uniform_buff, static_offsets<T>()[buffer_data_type::uniform]));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_texture_descriptors(ret.textures));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_pipeline_layout(logi_device));
            RESULT_VERIFY(ret.renderable_data_of<T>().create_pipeline(logi_device, window_render_pass));
            return error::unknown;
        };
        ((error_code = create_descriptors.template operator()<Ts>(error_code)), ...);
        if(error_code != error::unknown) return error_code;

        return ret;
    }
}

namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_tuple<FiF, std::tuple<Ts...>>::apply_changes(render_pass& window_render_pass) noexcept {
        constexpr static std::size_t I = renderable_index<T>;
        if(renderable_data_of<T>().size() == 0) {
            data_buffs[I] = buffer{};
            return {};
        }
        
        renderable_allocator allocator(logi_device_ptr, phys_device_ptr, copy_cmd_pool_ptr);
        auto load_texture = [this]<typename K>(K&& key) noexcept -> auto {
            return textures.load(std::forward<K>(key), logi_device_ptr, phys_device_ptr, font_data_map_ptr, copy_cmd_pool_ptr, texture_mem);
        };
        
        //Get the input data bytes, stage them, then move them to device local memory
        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> input_bytes, renderable_data_of<T>().make_inputs(load_texture), ib);
        RESULT_VERIFY_UNSCOPED(allocator.stage(renderable_data_of<T>().input_size(), input_bytes), s);
        renderable_data_of<T>().clear_inputs();
        auto [staging_buffer, staging_mem] = *std::move(s);

        if(renderable_data_of<T>().input_size() > data_buffs[I].size()) 
            RESULT_VERIFY((allocator.template alloc_buffer<I, renderable_data<T, FiF>::input_usage_flags(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(std::span{data_buffs}, renderable_data_of<T>().input_size(), device_local_mem)));
        RESULT_VERIFY((allocator.staging_to_device_local(data_buffs[I], staging_buffer)));

        if constexpr (renderable_constraints<T>::has_textures) {
            //TODO: Simplify this
            errc error_code = error::unknown;
            auto update_indicies = [&]<typename U>(errc& current_error_code) noexcept -> errc {
                if constexpr(!renderable_constraints<U>::has_textures) return current_error_code;
                else {
                    if(current_error_code != error::unknown) return current_error_code;
                    RESULT_VERIFY(renderable_data_of<U>().template update_texture_indices<T>(load_texture, allocator, data_buffs[renderable_index<U>]));
                    RESULT_VERIFY(renderable_data_of<U>().create_texture_descriptors(textures));
                    RESULT_VERIFY(renderable_data_of<U>().create_pipeline_layout(logi_device_ptr));
                    RESULT_VERIFY(renderable_data_of<U>().create_pipeline(logi_device_ptr, window_render_pass));
                    return error::unknown;
                }
            };
            ((error_code = update_indicies.template operator()<Ts>(error_code)), ...);
            (renderable_data_of<Ts>().clear_inputs(), ...);
            if(error_code != error::unknown) return error_code;
        }
        
        if constexpr (!renderable_constraints<T>::has_attributes) {
            renderable_data_of<T>().has_changes() = false;
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
        RESULT_TRY_COPY_UNSCOPED(void* shared_mem_map, shared_mem.map(logi_device_ptr, VK_WHOLE_SIZE), smm);
        std::size_t buffer_offset = 0;

        (renderable_data_of<Ts>().emplace_attributes(buffer_offset, shared_mem_map, shared_mem.requirements()[renderable_index_with_attrib<Ts>].size), ...);

        renderable_data_of<T>().has_changes() = false;
        return {};
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool const& renderable_tuple<FiF, std::tuple<Ts...>>::has_changes() const noexcept {
        return renderable_data_of<T>().has_changes();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool& renderable_tuple<FiF, std::tuple<Ts...>>::has_changes() noexcept {
        return renderable_data_of<T>().has_changes();
    }
}


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::end() noexcept {
        return renderable_data_of<T>().end();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::const_iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::end() const noexcept {
        return renderable_data_of<T>().end();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::const_iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::cend() const noexcept {
        return renderable_data_of<T>().cend();
    }
    

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::begin() noexcept {
        return renderable_data_of<T>().begin();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::const_iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::begin() const noexcept {
        return renderable_data_of<T>().begin();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    renderable_tuple<FiF, std::tuple<Ts...>>::const_iterator<T> renderable_tuple<FiF, std::tuple<Ts...>>::cbegin() const noexcept {
        return renderable_data_of<T>().cbegin();
    }
}

namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    bool renderable_tuple<FiF, std::tuple<Ts...>>::empty() const noexcept {
        return renderable_data_of<T>().empty();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::size() const noexcept {
        return renderable_data_of<T>().size();
    }
}




namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::index_buffer() const noexcept requires renderable_constraints<T>::has_indices { 
        if constexpr (renderable_constraints<T>::has_fixed_indices) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::uniform_buffer() const noexcept requires renderable_constraints<T>::has_uniform { 
        return uniform_buff; 
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::vertex_buffer() const noexcept requires renderable_constraints<T>::has_vertices  { 
        if constexpr (renderable_constraints<T>::has_fixed_vertices) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::instance_buffer() const noexcept requires renderable_constraints<T>::has_instance_data { 
        return data_buffs[renderable_index<T>]; 
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::attribute_buffer() const noexcept requires renderable_constraints<T>::has_attributes { 
        return attribute_buffs[renderable_index_with_attrib<T>]; 
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::texture_idx_buffer() const noexcept requires renderable_constraints<T>::has_textures { 
        return data_buffs[renderable_index<T>];
    };


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::index_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices) {
        return renderable_data_of<T>().index_count(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::vertex_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices) {
        return renderable_data_of<T>().vertex_count(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::instance_count() const noexcept { 
        return renderable_data_of<T>().instance_count();
    }


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::first_index(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices) {
        return renderable_data_of<T>().first_index(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::first_vertex(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices) {
        return renderable_data_of<T>().first_vertex(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::vertex_buffer_offset() const noexcept requires (renderable_constraints<T>::has_vertices) {
        if constexpr (renderable_constraints<T>::has_fixed_vertices) return static_offsets<T>()[buffer_data_type::vertex];
        else return renderable_data_of<T>().vertex_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::index_buffer_offset() const noexcept requires (renderable_constraints<T>::has_indices) {
        if constexpr (renderable_constraints<T>::has_fixed_indices) return static_offsets<T>()[buffer_data_type::index];
        else return renderable_data_of<T>().index_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::texture_idx_buffer_offset() const noexcept requires (renderable_constraints<T>::has_textures) {
        return renderable_data_of<T>().texture_idx_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_tuple<FiF, std::tuple<Ts...>>::static_offsets() noexcept { 
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


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline<T>& renderable_tuple<FiF, std::tuple<Ts...>>::associated_pipeline() const noexcept{ 
        return renderable_data_of<T>().pl;
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline_layout<T>& renderable_tuple<FiF, std::tuple<Ts...>>::associated_pipeline_layout() const noexcept{ 
        return renderable_data_of<T>().pl_layout;
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const std::array<VkDescriptorSet, FiF>& renderable_tuple<FiF, std::tuple<Ts...>>::desc_set() const noexcept{ 
        return renderable_data_of<T>().sets;
    }
}


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename renderable_properties<T>::uniform_type, FiF> renderable_tuple<FiF, std::tuple<Ts...>>::uniform_map() const noexcept requires (renderable_constraints<T>::has_uniform) { 
        return std::span<typename renderable_properties<T>::uniform_type, FiF>{reinterpret_cast<typename renderable_properties<T>::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + static_offsets<T>()[buffer_data_type::uniform]), FiF}; 
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<std::byte, 0> renderable_tuple<FiF, std::tuple<Ts...>>::uniform_map() const noexcept requires (!renderable_constraints<T>::has_uniform) { 
        return {}; 
    }
}