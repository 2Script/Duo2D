#pragma once
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Duo2D/traits/attribute_traits.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_set.hpp"
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"


//Attributes vs No Attributes
namespace d2d::impl {
    template<renderable_like T>
    class renderable_attribute_data {
    public:
        constexpr static std::size_t attribute_data_size = 0;
        constexpr static std::size_t num_attributes = 0;
    protected:
        std::unordered_map<std::string_view, T> input_renderables;

    public:
        std::size_t emplace_attributes(std::size_t& buff_offset, void*, VkDeviceSize) noexcept { return buff_offset; }

        constexpr std::size_t attribute_buffer_size() const noexcept { return 0; }
    };

    template<renderable_like T> requires T::has_attributes
    class renderable_attribute_data<T> {
    private:
        std::span<std::byte> attributes_span;
    protected:
        std::unordered_map<std::string_view, T> input_renderables;
    public:
        constexpr static std::size_t attribute_data_size = impl::attribute_traits<typename T::attribute_types>::total_size;
        constexpr static std::size_t num_attributes = std::tuple_size_v<decltype(std::declval<T>().attributes())>;
    
    private:
        template<std::size_t I>
        void emplace_single_attribute(std::array<std::size_t, num_attributes> attribute_offsets) noexcept;
    public:
        std::size_t emplace_attributes(std::size_t& buff_offset, void* mem_map, VkDeviceSize mem_align) noexcept;

        constexpr std::size_t attribute_buffer_size() const noexcept { return attribute_data_size * input_renderables.size(); }
    };
}


//Instanced vs Not Instanced
namespace d2d::impl {
    //Instanced
    template<impl::renderable_like T>
    class renderable_instance_data : public renderable_attribute_data<T> {
        using instance_input_type = decltype(std::declval<T>().instance());
    private:
        std::vector<instance_input_type> instance_inputs;
    protected:
        std::size_t input_data_size;
    public:
        constexpr static std::size_t instance_data_size = sizeof(typename T::instance_type);
        constexpr static std::size_t static_index_data_size = []() noexcept {
            if constexpr (T::has_fixed_indices) 
                return T::index_count * sizeof(typename T::index_type);
            return static_cast<std::size_t>(0); 
        }();
        constexpr static std::size_t static_vertex_data_size = []() noexcept{
            if constexpr (T::has_fixed_vertices)
                return T::vertex_count * sizeof(typename T::vertex_type);
            return static_cast<std::size_t>(0); 
        }();

        constexpr static auto static_index_data_bytes = []() noexcept {
            if constexpr (T::has_fixed_indices) 
                return std::bit_cast<std::array<std::byte, static_index_data_size>>(T::indices());
            return std::array<std::byte, static_index_data_size>{};
        }();
        constexpr static auto static_vertex_data_bytes = []() noexcept {
            if constexpr (T::has_fixed_vertices)
                return std::bit_cast<std::array<std::byte, static_vertex_data_size>>(T::vertices());
            return std::array<std::byte, static_vertex_data_size>{};
        }();
        

    public:
        result<std::vector<std::span<const std::byte>>> make_inputs() noexcept;

        constexpr std::size_t instance_count() const noexcept { return this->input_renderables.size(); }

        constexpr std::size_t input_size() const noexcept { return input_data_size; }
        consteval static VkBufferUsageFlags usage_flags() noexcept {
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | (T::has_indices ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0);
        }
    
    };

    //Non-instanced (no indices)
    template<impl::renderable_like T> requires (!T::instanced)
    class renderable_instance_data<T> : public renderable_attribute_data<T> {
        using vertex_input_type = decltype(std::declval<T>().vertices());
    private:
        std::vector<vertex_input_type> vertex_inputs;
        std::vector<std::uint32_t> vertex_firsts;
        std::vector<std::uint32_t> vertex_counts;
    protected:
        std::size_t vertex_offset = 0;
        std::size_t input_data_size;
    public:
        constexpr static std::size_t instance_data_size = 0;
        constexpr static std::size_t static_index_data_size = 0;
        constexpr static std::size_t static_vertex_data_size = 0;
        constexpr static std::array<std::byte, static_index_data_size> static_index_data_bytes = {};
        constexpr static std::array<std::byte, static_vertex_data_size> static_vertex_data_bytes = {};

    public:
        result<std::vector<std::span<const std::byte>>> make_inputs() noexcept;

        constexpr std::size_t instance_count() const noexcept { return this->input_renderables.size(); }
        constexpr std::size_t vertex_buffer_offset() const noexcept { return vertex_offset; }

        constexpr std::uint32_t vertex_count(std::size_t i) const noexcept { return vertex_counts[i]; }
        constexpr std::uint32_t first_vertex(std::size_t i) const noexcept { return vertex_firsts[i]; }

        constexpr std::size_t input_size() const noexcept { return input_data_size; }
        consteval static VkBufferUsageFlags usage_flags() noexcept { return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
    };
}

//Indices vs No Indices
namespace d2d::impl {
    template<impl::renderable_like T>
    class renderable_index_data : public renderable_instance_data<T> {};
    //template<impl::renderable_like T> requires (!T::instanced)
    //class renderable_index_data<T> : public renderable_instance_data<T> {};

    //Non-instanced with indices
    template<impl::renderable_like T> requires (!T::instanced && T::has_indices)
    class renderable_index_data<T> : public renderable_instance_data<T> {
        using index_input_type = decltype(std::declval<T>().indices());
    private:
        std::vector<index_input_type> index_inputs;
        std::vector<std::uint32_t> index_firsts;
        std::vector<std::uint32_t> index_counts;

    public:
        result<std::vector<std::span<const std::byte>>> make_inputs() noexcept;

        constexpr std::uint32_t index_count(std::size_t i) const noexcept { return index_counts[i]; }
        constexpr std::uint32_t first_index(std::size_t i) const noexcept { return index_firsts[i]; }

        consteval static VkBufferUsageFlags usage_flags() noexcept { 
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT; 
        }
    };

}


//Uniform vs No Uniform
namespace d2d::impl {
    template<renderable_like T, std::size_t FiF>
    class renderable_uniform_data {
    protected:
        pipeline_layout<T> pl_layout;
    public:
        constexpr static std::size_t uniform_data_size = 0;

    public:
        result<void> create_descriptors(logical_device& logi_device, buffer&, descriptor_pool<FiF>&, std::size_t) noexcept;
    };


    template<renderable_like T, std::size_t FiF> requires T::has_uniform
    class renderable_uniform_data<T, FiF> {
    protected:
        descriptor_set<FiF> set;
        descriptor_set_layout set_layout;
        pipeline_layout<T> pl_layout;
    public:
        constexpr static std::size_t uniform_data_size = FiF * sizeof(typename T::uniform_type);

    public:
        result<void> create_descriptors(logical_device& logi_device, buffer& uniform_buff, descriptor_pool<FiF>& desc_pool, std::size_t uniform_buff_offset) noexcept;
    };
}



namespace d2d {
    //Surely there's a better name for this class...
    template<impl::renderable_like T, std::size_t FiF>
    class renderable_data : public impl::renderable_index_data<T>, public impl::renderable_uniform_data<T, FiF> {
    private:
        bool outdated = false;
        pipeline<T> pl;
    
    public:
        result<void> create_pipeline(logical_device& logi_device, render_pass& window_render_pass) noexcept;

        template<std::size_t FramesInFlight, impl::renderable_like... Ts>
        friend struct renderable_tuple;
        friend struct window;
    };
}


#include "Duo2D/vulkan/memory/renderable_data.inl"