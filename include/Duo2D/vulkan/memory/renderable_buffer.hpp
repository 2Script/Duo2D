#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>
#include "Duo2D/graphics/prim/styled_rect.hpp"
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
        template<typename T>
        result<void> apply(bool shrink = false) noexcept;
        template<typename T>
        constexpr bool needs_apply() const noexcept;

    private:
        template<typename T, VkBufferUsageFlags BufferFlag>
        result<void> create_buffer(bool shrink, std::size_t size, buffer& buff, device_memory& mem) noexcept;
        template<typename T, VkBufferUsageFlags BufferFlag>
        result<void*> create_shared_buffer(bool shrink, std::size_t size, buffer& buff, device_memory& mem) noexcept;
        template<typename T> 
        result<void> create_pipelines(logical_device& logi_device, render_pass& window_render_pass) noexcept;

        result<void> copy(buffer& dst, const buffer& src, std::size_t size) const noexcept;
        template<typename T>
        static void copy_staging(void* data_map) noexcept;

    public:
        template<typename U> requires impl::RenderableType<std::remove_cvref_t<U>>
        constexpr void push_back(U&& value) noexcept;
        template<typename T, typename... Args>
        constexpr T& emplace_back(Args&&... args) noexcept;

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
        struct bytes_t {
            std::size_t index;
            std::size_t uniform;
            std::size_t vertex = 0;
            std::size_t instance = 0;
            
            friend constexpr bytes_t operator+(bytes_t a, const bytes_t& b) noexcept { 
                return {a.index + b.index, a.uniform + b.uniform, a.vertex + b.vertex, a.instance + b.instance}; 
            }
        };

        constexpr static std::size_t count               = impl::type_count<Ts...>::value;
        constexpr static std::size_t indexed_count       = impl::type_count<Ts...>::indexed_value;
        constexpr static std::size_t uniformed_count     = impl::type_count<Ts...>::uniformed_value;
        constexpr static std::size_t attributed_count    = impl::type_count<Ts...>::attributed_value;
        template<typename T> constexpr static std::size_t idx               = impl::type_index<T, Ts...>::value; 
        template<typename T> constexpr static std::size_t indexed_idx       = impl::type_index<T, Ts...>::indexed_value; 
        template<typename T> constexpr static std::size_t uniformed_idx     = impl::type_index<T, Ts...>::uniformed_value; 
        template<typename T> constexpr static std::size_t attribute_idx     = impl::type_index<T, Ts...>::attributed_value; 
        static_assert(attributed_count == std::tuple_size_v<impl::attributes_tuple<Ts...>>);
        
        template<typename T> using attribute_types = typename T::attribute_types;
        template<typename T> using index_type = typename T::index_type;
        template<typename T> using uniform_type = typename T::uniform_type;
        template<typename T> using push_constant_types = typename T::push_constant_types;
        template<typename T> using vertex_type = typename T::vertex_type;
        template<typename T> using instance_type = typename T::instance_type;
        template<typename T> constexpr static bytes_t data_size = {
            .index = T::index_count * sizeof(index_type<T>),
            .uniform = [](){ if constexpr (impl::has_uniform_v<T>) return sizeof(uniform_type<T>); return static_cast<std::size_t>(0); }(),
            .vertex = [](){ if constexpr (impl::has_vertices_v<T>) return (T::vertex_count * sizeof(vertex_type<T>)); return static_cast<std::size_t>(0); }(),
            .instance = [](){ if constexpr (T::instanced) return sizeof(instance_type<T>); return static_cast<std::size_t>(0); }(),
        };
        constexpr static bytes_t instanced_buffer_size = ((Ts::instanced ? data_size<Ts> : bytes_t{}) + ...); 
        template<typename T> constexpr static std::size_t num_attributes = std::tuple_size_v<decltype(std::declval<T>().attributes())>;
        

    private:
        template<std::size_t I, typename T>
        constexpr void emplace_attribute(std::vector<T>& data_vec, std::span<std::byte> attributes_span, std::array<std::size_t, num_attributes<T>> attribute_offsets, std::size_t attribute_size) noexcept;

    public:
        template<typename T> constexpr const buffer& index_buffer() const noexcept requires impl::has_indices_v<T>;
        template<typename T> constexpr const buffer& uniform_buffer() const noexcept requires impl::has_uniform_v<T>;
        template<typename T> constexpr const buffer& vertex_buffer() const noexcept requires impl::has_vertices_v<T>;
        template<typename T> constexpr const buffer& instance_buffer() const noexcept requires (T::instanced);
        template<typename T> constexpr const buffer& attribute_buffer() const noexcept requires impl::has_attributes_v<T>;

        template<typename T> constexpr std::size_t index_count() const noexcept;
        template<typename T> constexpr std::size_t vertex_count() const noexcept;
        template<typename T> constexpr std::size_t instance_count() const noexcept;

        template<typename T> consteval static bytes_t offsets() noexcept;


        //template<typename T> constexpr const attribute_types<T>& attributes() const noexcept;
        template<typename T> constexpr std::span<uniform_type<T>> uniform_map() const noexcept requires impl::has_uniform_v<T>;
        template<typename T> constexpr push_constant_types<T> push_constants() const noexcept requires impl::has_push_constants_v<T>;

        template<typename T> constexpr const pipeline<T>& associated_pipeline() const noexcept;
        template<typename T> constexpr const pipeline_layout<T>& associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const descriptor_set<FramesInFlight>& desc_set() const noexcept;


    private:
        std::tuple<std::vector<Ts>...> data;
        impl::attributes_tuple<Ts...> attributes_map_tuple; //tuple<span<T0_bytes>, span<T1_bytes>, ...>
        std::array<bool, count> outdated;

        //up to (3 * sizeof...(Ts) + 2) total memory allocations
        buffer static_data_buff;
        device_memory static_data_mem;
        buffer uniform_buff;
        device_memory uniform_mem;
        void* uniform_buffer_map;
        std::array<buffer, indexed_count> index_buffs;
        std::array<device_memory, indexed_count> index_mems;
        std::array<buffer, attributed_count> attribute_buffs; //TODO allow using SSBOs instead
        std::array<device_memory, attributed_count> attribute_mems;
        std::array<buffer, count> instance_buffs;
        std::array<device_memory, count> instance_mems;
        
        std::tuple<pipeline<Ts>...> pipelines;
        std::tuple<pipeline_layout<Ts>...> pipeline_layouts;

        descriptor_pool<FramesInFlight, uniformed_count> desc_pool;
        std::array<descriptor_set<FramesInFlight>, uniformed_count> desc_sets;
        std::array<descriptor_set_layout, uniformed_count> desc_set_layouts;

        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        command_pool copy_cmd_pool;

    private:
        consteval static std::array<bytes_t, count> make_offsets() noexcept { 
            std::array<bytes_t, count> ret;
            bytes_t last_instanced_offset = {};
            bytes_t last_instanced_size = {};
            std::size_t last_uniform_size = 0;
            (((ret[idx<Ts>] = (idx<Ts> == 0 ? bytes_t{
                .vertex = Ts::instanced ? data_size<Ts>.index : 0,
            } : bytes_t{
                .index = Ts::instanced ? (last_instanced_offset.vertex + last_instanced_size.vertex) : 0,
                .uniform = ret[idx<Ts> - 1].uniform + last_uniform_size,
                .vertex = Ts::instanced ? (last_instanced_offset.vertex + last_instanced_size.vertex + data_size<Ts>.index) : 0,
            })), 
            (last_instanced_offset = (Ts::instanced ? ret[idx<Ts>] : last_instanced_offset)),
            (last_instanced_size = (Ts::instanced ? data_size<Ts> : last_instanced_size)),
            (last_uniform_size = data_size<Ts>.uniform * FramesInFlight)),...);
            return ret; 
        }

        constexpr static std::array<bytes_t, count> buff_offsets = make_offsets();
    };
}

#include "Duo2D/vulkan/memory/renderable_buffer.inl"