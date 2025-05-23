#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <tuple>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_set.hpp"
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "zstring.hpp"

namespace d2d {
    template<std::size_t FramesInFlight, impl::renderable_like... Ts> //requires (sizeof...(Ts) > 0)
    struct renderable_tuple {
        static_assert(sizeof...(Ts) > 0, "renderable_tuple needs at least 1 renderable type");
    public:
        //could benefit from SIMD? (just a target_clones)
        static result<renderable_tuple> create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept;
    
    public:
        template<typename U> requires impl::renderable_like<std::remove_cvref_t<U>>
        constexpr void push_back(U&& value) noexcept;
        template<typename T, typename... Args>
        constexpr T& emplace_back(Args&&... args) noexcept;

        //TODO: rename/refactor to apply_changes
        template<typename T>
        result<void> apply() noexcept;
        //TODO: rename/refactor to has_changes
        template<typename T>
        constexpr bool needs_apply() const noexcept;

        template<typename T>
        constexpr typename std::vector<T>::iterator erase(typename std::vector<T>::const_iterator pos) noexcept;
        template<typename T>
        constexpr typename std::vector<T>::iterator erase(typename std::vector<T>::const_iterator first, typename std::vector<T>::const_iterator last) noexcept;

        template<typename T>
        constexpr typename std::vector<T>::iterator begin() noexcept;
        template<typename T>
        constexpr typename std::vector<T>::iterator end() noexcept;
        template<typename T>
        constexpr typename std::vector<T>::const_iterator cbegin() const noexcept;
        template<typename T>
        constexpr typename std::vector<T>::const_iterator cend() const noexcept;
        
        template<typename T>
        constexpr bool empty() const noexcept;
        template<typename T>
        constexpr std::size_t size() const noexcept;

    public:
        template<typename T> constexpr const buffer& index_buffer() const noexcept requires T::has_indices;
        template<typename T> constexpr const buffer& uniform_buffer() const noexcept requires T::has_uniform;
        template<typename T> constexpr const buffer& vertex_buffer() const noexcept requires T::has_vertices;
        template<typename T> constexpr const buffer& instance_buffer() const noexcept requires (T::instanced);
        template<typename T> constexpr const buffer& attribute_buffer() const noexcept requires T::has_attributes;


        template<typename T> constexpr std::uint32_t index_count(std::size_t i) const noexcept requires (!T::instanced && T::has_indices);
        template<typename T> constexpr std::uint32_t vertex_count(std::size_t i) const noexcept requires (!T::instanced && T::has_vertices);
        template<typename T> constexpr std::size_t instance_count() const noexcept;

        template<typename T> constexpr std::uint32_t first_index(std::size_t i) const noexcept requires (!T::instanced && T::has_indices); 
        template<typename T> constexpr std::uint32_t first_vertex(std::size_t i) const noexcept requires (!T::instanced && T::has_vertices); 
        template<typename T> constexpr std::size_t vertex_buffer_offset() const noexcept requires (!T::instanced && T::has_vertices); 
        template<typename T> consteval static buffer_bytes_t static_offsets() noexcept;


        //template<typename T> constexpr const attribute_types<T>& attributes() const noexcept;
        template<typename T> constexpr std::span<typename T::uniform_type> uniform_map() const noexcept requires T::has_uniform;
        template<typename T> constexpr typename T::push_constant_types push_constants() const noexcept requires T::has_push_constants;

        template<typename T> constexpr const pipeline<T>& associated_pipeline() const noexcept;
        template<typename T> constexpr const pipeline_layout<T>& associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const descriptor_set<FramesInFlight>& desc_set() const noexcept;



    private:
        template<typename T>
        constexpr renderable_data<T, FramesInFlight>& renderable_data_of() noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }
        template<typename T>
        constexpr renderable_data<T, FramesInFlight> const& renderable_data_of() const noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }

    private:
        template<typename InputContainerT>
        result<std::pair<buffer, device_memory<1>>> stage(std::size_t total_buffer_size, InputContainerT&& inputs) noexcept;
        template<std::size_t I, VkFlags BufferUsage, VkMemoryPropertyFlags MemProps, VkMemoryPropertyFlags FallbackMemProps, std::size_t N>
        result<void> alloc_buffer(std::span<buffer, N> buffs, std::size_t total_buffer_size, device_memory<N>& mem) noexcept;
        template<std::size_t I, std::size_t N>
        result<void> staging_to_device_local(std::span<buffer, N> device_local_buffs, buffer const& staging_buff) noexcept;
        

    private:
        std::tuple<renderable_data<Ts, FramesInFlight>...> renderable_datas;

        constexpr static std::size_t renderable_count = sizeof...(Ts);
        constexpr static std::size_t renderable_count_with_attrib = (static_cast<bool>(renderable_data<Ts, FramesInFlight>::attribute_data_size) + ...);
        //TODO: Find a better strategy than these manual index calculation shenanigans
        template<typename T>
        constexpr static std::size_t renderable_index = impl::type_index<T, Ts...>::value;
        template<typename T>
        constexpr static std::size_t renderable_index_with_attrib = impl::type_index<T, Ts...>::with_attrib_value;


        //up to 4 total memory allocations
        //ORDER MATTERS: buffers must be destroyed before memories
        device_memory<renderable_count> device_local_mem; //used for vertex and index data of non-instanced types
        device_memory<1> static_device_local_mem; //used for vertex and index data of instanced types
        device_memory<1> host_mem; //used for uniform
        device_memory<renderable_count_with_attrib> shared_mem; //used for attributes

        std::array<buffer, renderable_count> data_buffs;
        buffer static_data_buff;
        buffer uniform_buff;
        std::array<buffer, renderable_count_with_attrib> attribute_buffs;
        void* uniform_buffer_map;

        descriptor_pool<FramesInFlight> desc_pool;

        //TODO use std::reference_wrapper to better show intent?
        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        command_pool copy_cmd_pool;
    };
}

#include "Duo2D/vulkan/memory/renderable_tuple.inl"