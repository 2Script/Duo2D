#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/device_memory.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/device_memory.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/pipeline_layout.hpp"
#include "Duo2D/graphics/prim/renderable_traits.hpp"

namespace d2d::impl {
    template <typename T, typename U, typename... Ts>
    struct type_index : std::integral_constant<std::size_t, 1 + type_index<T, Ts...>::value> {
        constexpr static std::size_t value = 1 + type_index<T, Ts...>::value;
        constexpr static std::size_t indexed_value = (!U::instanced && impl::IndexRenderableType<U>) + type_index<T, Ts...>::indexed_value;
        constexpr static std::size_t uniformed_value = (impl::UniformRenderableType<U>) + type_index<T, Ts...>::uniformed_value;
    };
    template <typename T, typename... Ts>
    struct type_index<T, T, Ts...> {
        constexpr static std::size_t value = 0;
        constexpr static std::size_t indexed_value = 0;
        constexpr static std::size_t uniformed_value = 0;
    };

    template<typename... Ts>
    struct type_count {
        constexpr static std::size_t value = sizeof...(Ts);
        constexpr static std::size_t indexed_value = ((!Ts::instanced && impl::IndexRenderableType<Ts>) + ...);
        constexpr static std::size_t uniformed_value = ((impl::UniformRenderableType<Ts>) + ...);
    };
}

namespace d2d {
    template<std::size_t FramesInFlight, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    struct renderable_buffer : std::tuple<std::vector<Ts>...> {
        //could benefit from SIMD (just a target_clones)
        static result<renderable_buffer> create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept;

    private:
        result<void> copy(buffer& dst, const buffer& src, std::size_t size) const noexcept;
        template<typename T>
        static void copy_staging(void* data_map) noexcept;

    public:
        template<typename T>
        result<void> apply(bool shrink = false) noexcept;
    private:
        template<typename T, VkBufferUsageFlags BufferFlag>
        result<void> create_buffer(bool shrink, std::size_t size, buffer& buff, device_memory& mem) noexcept;
        template<typename T> 
        result<void> create_pipelines(logical_device& logi_device, render_pass& window_render_pass) noexcept;

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

        constexpr static std::size_t count = impl::type_count<Ts...>::value;
        constexpr static std::size_t indexed_count = impl::type_count<Ts...>::indexed_value;
        constexpr static std::size_t uniformed_count = impl::type_count<Ts...>::uniformed_value;
        template<typename T> constexpr static std::size_t idx = impl::type_index<T, Ts...>::value; 
        template<typename T> constexpr static std::size_t indexed_idx = impl::type_index<T, Ts...>::indexed_value; 
        template<typename T> constexpr static std::size_t uniformed_idx = impl::type_index<T, Ts...>::uniformed_value; 
        
        template<typename T> using index_type = typename T::index_type;
        template<typename T> using uniform_type = typename T::uniform_type;
        template<typename T> using vertex_type = typename T::vertex_type;
        template<typename T> using instance_type = typename T::instance_type;
        template<typename T> constexpr static bytes_t data_size = {
            .index = T::index_count * sizeof(index_type<T>),
            .uniform = [](){ if constexpr (impl::UniformRenderableType<T>) return sizeof(uniform_type<T>); return static_cast<std::size_t>(0); }(),
            .vertex = [](){ if constexpr (impl::VertexRenderableType<T>) return (T::vertex_count * sizeof(vertex_type<T>)); return static_cast<std::size_t>(0); }(),
            .instance = [](){ if constexpr (T::instanced) return sizeof(instance_type<T>); return static_cast<std::size_t>(0); }(),
        };
        constexpr static bytes_t instanced_buffer_size = ((Ts::instanced ? data_size<Ts> : bytes_t{}) + ...); 
        

    public:
        template<typename T> constexpr const buffer& index_buffer() const noexcept requires impl::IndexRenderableType<T> { 
            if constexpr (T::instanced) return static_data_buff; 
            else return index_buffs[indexed_idx<T>]; 
        }
        template<typename T> constexpr const buffer& uniform_buffer() const noexcept requires impl::UniformRenderableType<T> { 
            return uniform_buff; 
        }
        template<typename T> constexpr const buffer& vertex_buffer() const noexcept requires impl::VertexRenderableType<T>  { 
            if constexpr (T::instanced) return static_data_buff; 
            else return instance_buffs[idx<T>]; 
        };
        template<typename T> constexpr const buffer& instance_buffer() const noexcept requires (T::instanced) { 
            return instance_buffs[idx<T>]; 
        };

        template<typename T> constexpr std::size_t index_count() const noexcept {
            std::size_t idx_cnt = T::index_count;
            if constexpr (!T::instanced) idx_cnt *= std::get<std::vector<T>>(*this).size(); 
            return idx_cnt;
        }
        template<typename T> constexpr std::size_t vertex_count() const noexcept {
            std::size_t vert_cnt = T::vertex_count;
            if constexpr (!T::instanced) vert_cnt *= std::get<std::vector<T>>(*this).size(); 
            return vert_cnt;
        }
        template<typename T> constexpr std::size_t instance_count() const noexcept { 
            if constexpr (T::instanced) return std::get<std::vector<T>>(*this).size(); 
            else return 1;
        }

        template<typename T> constexpr std::span<uniform_type<T>> uniform_map() const noexcept requires impl::UniformRenderableType<T> { 
            return {reinterpret_cast<uniform_type<T>*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + buff_offsets[idx<T>].uniform), FramesInFlight}; 
        }
        template<typename T> constexpr const pipeline<T>& associated_pipeline() const noexcept{ 
            return std::get<pipeline<T>>(pipelines);
        }
        template<typename T> constexpr const pipeline_layout& associated_pipeline_layout() const noexcept{ 
            return pipeline_layouts[idx<T>];
        }
        template<typename T> constexpr const descriptor_set<FramesInFlight>& desc_set() const noexcept{ 
            return desc_sets[uniformed_idx<T>];
        }

        template<typename T> consteval static bytes_t offsets() noexcept { return buff_offsets[idx<T>]; }

    private:
        //(count * non_instanced_count + 2) total memory allocations
        buffer static_data_buff;
        device_memory static_data_mem;
        buffer uniform_buff;
        device_memory uniform_mem;
        void* uniform_buffer_map;
        std::array<buffer, indexed_count> index_buffs;
        std::array<device_memory, indexed_count> index_mems;
        std::array<buffer, count> instance_buffs;
        std::array<device_memory, count> instance_mems;
        
        std::tuple<pipeline<Ts>...> pipelines;
        std::array<pipeline_layout, count> pipeline_layouts;

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

#include "Duo2D/graphics/pipeline/renderable_buffer.inl"