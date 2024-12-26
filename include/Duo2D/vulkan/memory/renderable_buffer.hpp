#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>
#include "Duo2D/graphics/prim/styled_rect.hpp"
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
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

namespace d2d {
    template<std::size_t FramesInFlight, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    struct renderable_buffer {
        //could benefit from SIMD (just a target_clones)
        static result<renderable_buffer> create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept;
    
    public:
        template<typename U> requires impl::RenderableType<std::remove_cvref_t<U>>
        constexpr void push_back(U&& value) noexcept;
        template<typename T, typename... Args>
        constexpr T& emplace_back(Args&&... args) noexcept;

        template<typename T>
        result<void> apply(bool shrink = false) noexcept;
        template<typename T>
        constexpr bool needs_apply() const noexcept;

        template<typename T>
        constexpr typename std::vector<T>::iterator erase(typename std::vector<T>::const_iterator pos) noexcept;
        template<typename T>
        constexpr typename std::vector<T>::iterator erase(typename std::vector<T>::const_iterator first, typename std::vector<T>::const_iterator last) noexcept;

        template<typename T>
        constexpr typename std::vector<T>::const_iterator cbegin() const noexcept;
        template<typename T>
        constexpr typename std::vector<T>::const_iterator cend() const noexcept;
        
        template<typename T>
        constexpr bool empty() const noexcept;
        template<typename T>
        constexpr std::size_t size() const noexcept;

    public:
        template<typename T> constexpr const buffer& index_buffer() const noexcept requires impl::has_indices_v<T>;
        template<typename T> constexpr const buffer& uniform_buffer() const noexcept requires impl::has_uniform_v<T>;
        template<typename T> constexpr const buffer& vertex_buffer() const noexcept requires impl::has_vertices_v<T>;
        template<typename T> constexpr const buffer& instance_buffer() const noexcept requires (T::instanced);
        template<typename T> constexpr const buffer& attribute_buffer() const noexcept requires impl::has_attributes_v<T>;

        template<typename T> constexpr std::size_t index_count() const noexcept;
        template<typename T> constexpr std::size_t vertex_count() const noexcept;
        template<typename T> constexpr std::size_t instance_count() const noexcept;

        template<typename T> consteval static buffer_bytes_t offsets() noexcept;

        //template<typename T> constexpr const attribute_types<T>& attributes() const noexcept;
        template<typename T> constexpr std::span<typename T::uniform_type> uniform_map() const noexcept requires impl::has_uniform_v<T>;
        template<typename T> constexpr typename T::push_constant_types push_constants() const noexcept requires impl::has_push_constants_v<T>;

        template<typename T> constexpr const pipeline<T>& associated_pipeline() const noexcept;
        template<typename T> constexpr const pipeline_layout<T>& associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const descriptor_set<FramesInFlight>& desc_set() const noexcept;



    private:

        constexpr static std::array<std::size_t, impl::type_filter::count> count_v = impl::type_count<Ts...>::value;
        template<typename T> constexpr static std::array<std::size_t, impl::type_filter::count> index_v = impl::type_index<T, Ts...>::value;
        static_assert(count_v[impl::type_filter::has_attrib] == std::tuple_size_v<impl::attributes_tuple<Ts...>>);
        
        template<typename T> constexpr static buffer_bytes_t data_size = {
            [](){if constexpr (impl::has_indices_v<T>)  return T::index_count * sizeof(typename T::index_type);   return static_cast<std::size_t>(0); }(),
            [](){if constexpr (impl::has_uniform_v<T>)  return FramesInFlight * sizeof(typename T::uniform_type); return static_cast<std::size_t>(0); }(),
            [](){if constexpr (impl::has_vertices_v<T>) return T::vertex_count * sizeof(typename T::vertex_type); return static_cast<std::size_t>(0); }(),
            [](){if constexpr (T::instanced)            return sizeof(typename T::instance_type);                 return static_cast<std::size_t>(0); }(), 
        };
        constexpr static buffer_bytes_t instanced_buffer_size = ((Ts::instanced ? data_size<Ts> : buffer_bytes_t{}) + ...); 
        template<typename T> constexpr static std::size_t num_attributes = std::tuple_size_v<decltype(std::declval<T>().attributes())>;
        

    private:
        template<typename T, impl::type_filter::idx BuffFilter, std::size_t N>
        result<void> create_buffer(bool shrink, std::size_t new_size, std::array<buffer, N>& buffs, device_memory<N>& mem) noexcept;
        template<typename T, impl::type_filter::idx BuffFilter, std::size_t N>
        result<void*> create_shared_buffer(bool shrink, std::size_t new_size, std::array<buffer, N>& buffs) noexcept;
        template<typename T> 
        result<void> create_pipelines(logical_device& logi_device, render_pass& window_render_pass) noexcept;

        result<void> copy(buffer& dst, const buffer& src, std::size_t size) const noexcept;
        template<typename T>
        static void copy_staging(void* data_map) noexcept;

        template<std::size_t I, typename T>
        constexpr void emplace_attribute(std::vector<T>& data_vec, std::span<std::byte> attributes_span, std::array<std::size_t, num_attributes<T>> attribute_offsets, std::size_t attribute_size) noexcept;

    private:
        std::tuple<std::vector<Ts>...> data;
        impl::attributes_tuple<Ts...> attributes_map_tuple; //tuple<span<T0_bytes>, span<T1_bytes>, ...>
        std::array<bool, count_v[impl::type_filter::none]> outdated;

        //up to 5 total memory allocations
        //ORDER MATTERS: buffers must be destroyed before memories
        device_memory<1> static_data_mem;
        device_memory<1> host_mem;
        device_memory<count_v[impl::type_filter::has_index]> index_device_local_mem;
        device_memory<count_v[impl::type_filter::none]> data_device_local_mem;
        device_memory<count_v[impl::type_filter::has_attrib]> shared_mem;
        std::array<buffer, count_v[impl::type_filter::has_index]> index_buffs;
        std::array<buffer, count_v[impl::type_filter::none]> data_buffs;
        std::array<buffer, count_v[impl::type_filter::has_attrib]> attribute_buffs;
        buffer static_data_buff;
        buffer uniform_buff;
        void* uniform_buffer_map;
        
        std::tuple<pipeline<Ts>...> pipelines;
        std::tuple<pipeline_layout<Ts>...> pipeline_layouts;

        descriptor_pool<FramesInFlight, count_v[impl::type_filter::has_uniform]> desc_pool;
        std::array<descriptor_set<FramesInFlight>, count_v[impl::type_filter::has_uniform]> desc_sets;
        std::array<descriptor_set_layout, count_v[impl::type_filter::has_uniform]> desc_set_layouts;

        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        command_pool copy_cmd_pool;
    };
}

#include "Duo2D/vulkan/memory/renderable_buffer.inl"