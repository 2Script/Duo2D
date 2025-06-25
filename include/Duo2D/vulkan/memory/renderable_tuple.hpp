#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.h>
#include <zstring.hpp>
#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

#include "Duo2D/graphics/prim/styled_rect.hpp"

namespace d2d {
    template<std::size_t FramesInFlight, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    struct renderable_tuple {
        static_assert(sizeof...(Ts) > 0, "renderable_tuple needs at least 1 renderable type");
    public:
        //could benefit from SIMD? (just a target_clones)
        static result<renderable_tuple> create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept;
    
    public:
        //TODO: rename/refactor to apply_changes
        template<typename T>
        result<void> apply(render_pass& window_render_pass) noexcept;
        //TODO: rename/refactor to has_changes
        template<typename T>
        constexpr bool needs_apply() const noexcept;
        template<typename T>
        constexpr bool empty() const noexcept;

    public:
        template<typename T> constexpr const buffer& index_buffer() const noexcept requires renderable_constraints<T>::has_indices;
        template<typename T> constexpr const buffer& uniform_buffer() const noexcept requires renderable_constraints<T>::has_uniform;
        template<typename T> constexpr const buffer& vertex_buffer() const noexcept requires renderable_constraints<T>::has_vertices;
        template<typename T> constexpr const buffer& instance_buffer() const noexcept requires (renderable_constraints<T>::instanced);
        template<typename T> constexpr const buffer& attribute_buffer() const noexcept requires renderable_constraints<T>::has_attributes;
        template<typename T> constexpr const buffer& texture_idx_buffer() const noexcept requires renderable_constraints<T>::has_textures;


        template<typename T> constexpr std::uint32_t index_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices);
        template<typename T> constexpr std::uint32_t vertex_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_vertices);
        template<typename T> constexpr std::size_t instance_count() const noexcept;

        template<typename T> constexpr std::uint32_t first_index(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices); 
        template<typename T> constexpr std::uint32_t first_vertex(std::size_t i) const noexcept requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_vertices); 

        template<typename T> constexpr std::size_t vertex_buffer_offset() const noexcept requires (renderable_constraints<T>::has_vertices); 
        template<typename T> constexpr std::size_t index_buffer_offset() const noexcept requires (renderable_constraints<T>::has_indices); 
        template<typename T> constexpr std::size_t texture_idx_buffer_offset() const noexcept requires (renderable_constraints<T>::has_textures); 
        template<typename T> consteval static buffer_bytes_t static_offsets() noexcept;


        //template<typename T> constexpr const attribute_types<T>& attributes() const noexcept;
        template<typename T> constexpr std::span<typename T::uniform_type> uniform_map() const noexcept requires renderable_constraints<T>::has_uniform;
        template<typename T> constexpr typename T::push_constant_types push_constants() const noexcept requires renderable_constraints<T>::has_push_constants;

        template<typename T> constexpr const pipeline<T>& associated_pipeline() const noexcept;
        template<typename T> constexpr const pipeline_layout<T>& associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const std::array<VkDescriptorSet, FramesInFlight>& desc_set() const noexcept;



    private:
        template<typename T>
        constexpr renderable_data<T, FramesInFlight>& renderable_data_of() noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }
        template<typename T>
        constexpr renderable_data<T, FramesInFlight> const& renderable_data_of() const noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }

        friend struct window;

    private:
        std::tuple<renderable_data<Ts, FramesInFlight>...> renderable_datas;

        constexpr static std::size_t renderable_count = sizeof...(Ts);
        constexpr static std::size_t renderable_count_with_attrib = (static_cast<bool>(renderable_data<Ts, FramesInFlight>::attribute_data_size) + ...);
        constexpr static std::size_t renderable_count_with_textures = (renderable_constraints<Ts>::has_textures + ...);
        //TODO: Find a better strategy than these manual index calculation shenanigans
        template<typename T>
        constexpr static std::size_t renderable_index = impl::type_index<T, Ts...>::value;
        template<typename T>
        constexpr static std::size_t renderable_index_with_attrib = impl::type_index<T, Ts...>::with_attrib_value;
        template<typename T>
        constexpr static std::size_t renderable_index_with_textures = impl::type_index<T, Ts...>::with_textures_value;


        //up to 5 total memory allocations
        //ORDER MATTERS: buffers must be destroyed before memories
        device_memory<renderable_count> device_local_mem; //used for vertex and index data of non-instanced types
        device_memory<std::dynamic_extent> texture_mem; //used for texture data
        device_memory<1> static_device_local_mem; //used for vertex and index data of instanced types
        device_memory<1> host_mem; //used for uniform
        device_memory<renderable_count_with_attrib> shared_mem; //used for attributes

        texture_map textures;
        buffer texture_size_buffer;
        std::array<buffer, renderable_count> data_buffs;
        buffer static_data_buff;
        std::array<buffer, renderable_count_with_attrib> attribute_buffs;
        buffer uniform_buff;
        void* uniform_buffer_map;

        //TODO use std::reference_wrapper to better show intent?
        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        command_pool copy_cmd_pool;

        //renderable_allocator allocator;
    };
}

#include "Duo2D/vulkan/memory/renderable_tuple.inl"