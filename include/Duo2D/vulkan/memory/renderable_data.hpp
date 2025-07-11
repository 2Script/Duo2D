#pragma once
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/traits/attribute_traits.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"

namespace d2d::vk::impl {
    template<typename T>
    using renderable_input_map = std::unordered_map<std::string, T>;
}


namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    class renderable_input_data : public renderable_input_map<T> {
    public:
        using key_type       = typename renderable_input_map<T>::key_type;
        using mapped_type    = typename renderable_input_map<T>::mapped_type;
        using value_type     = typename renderable_input_map<T>::value_type;
        using iterator       = typename renderable_input_map<T>::iterator;
        using const_iterator = typename renderable_input_map<T>::const_iterator;
    public:
        iterator erase(iterator pos) noexcept;
        iterator erase(const_iterator pos) noexcept;
        iterator erase(const_iterator first, const_iterator last) noexcept;
        std::size_t erase(std::string_view key) noexcept;

    public:
        constexpr bool const& has_changes() const noexcept { return outdated; }
        constexpr bool      & has_changes()       noexcept { return outdated; }

    private:
        bool outdated = false;
    };
}


//Attributes vs No Attributes
namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    class renderable_attribute_data : public renderable_input_data<T>  {
    public:
        constexpr static std::size_t attribute_data_size = 0;
        constexpr static std::size_t num_attributes = 0;

    public:
        std::size_t emplace_attributes(std::size_t& buff_offset, void*, VkDeviceSize) noexcept { return buff_offset; }
        void unbind_attributes() noexcept {};

        constexpr std::size_t attribute_buffer_size() const noexcept { return 0; }
    };

    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_attributes
    class renderable_attribute_data<T> : public renderable_input_data<T> {
    private:
        std::span<std::byte> attributes_span;
    public:
        constexpr static std::size_t attribute_data_size = impl::attribute_traits<typename renderable_traits<T>::attribute_types>::total_size;
        constexpr static std::size_t num_attributes = std::tuple_size_v<decltype(std::declval<T>().attributes())>;
    
    private:
        template<std::size_t I>
        void emplace_single_attribute(std::array<std::size_t, num_attributes> attribute_offsets) noexcept;
        template<std::size_t I>
        void unbind_single_attribute() noexcept;
    public:
        std::size_t emplace_attributes(std::size_t& buff_offset, void* mem_map, VkDeviceSize mem_align) noexcept;
        void unbind_attributes() noexcept;

        constexpr std::size_t attribute_buffer_size() const noexcept { return attribute_data_size * this->size(); }
    };
}


//Fixed vs Not Fixed 
namespace d2d::vk::impl {
    //Fixed
    template<::d2d::impl::directly_renderable T>
    class renderable_fixed_data : public renderable_attribute_data<T> {
    public:
        constexpr static std::size_t static_index_data_size = []() noexcept {
            if constexpr (renderable_constraints<T>::has_fixed_indices) 
                return renderable_traits<T>::index_count * sizeof(typename renderable_traits<T>::index_type);
            return static_cast<std::size_t>(0); 
        }();
        constexpr static std::size_t static_vertex_data_size = []() noexcept{
            if constexpr (renderable_constraints<T>::has_fixed_vertices)
                return renderable_traits<T>::vertex_count * sizeof(typename renderable_traits<T>::vertex_type);
            return static_cast<std::size_t>(0); 
        }();

        constexpr static auto static_index_data_bytes = []() noexcept {
            if constexpr (renderable_constraints<T>::has_fixed_indices) 
                return std::bit_cast<std::array<std::byte, static_index_data_size>>(T::indices());
            return std::array<std::byte, static_index_data_size>{};
        }();
        constexpr static auto static_vertex_data_bytes = []() noexcept {
            if constexpr (renderable_constraints<T>::has_fixed_vertices)
                return std::bit_cast<std::array<std::byte, static_vertex_data_size>>(T::vertices());
            return std::array<std::byte, static_vertex_data_size>{};
        }();
        

    public:
        constexpr std::size_t instance_count() const noexcept { return this->size(); }
    };


    //Not Fixed
    template<::d2d::impl::directly_renderable T> requires (!renderable_constraints<T>::has_fixed_vertices && !renderable_constraints<T>::has_fixed_indices)
    class renderable_fixed_data<T> : public renderable_attribute_data<T> {
    public:
        constexpr static std::size_t static_index_data_size = 0;
        constexpr static std::size_t static_vertex_data_size = 0;
        constexpr static std::array<std::byte, static_index_data_size> static_index_data_bytes = {};
        constexpr static std::array<std::byte, static_vertex_data_size> static_vertex_data_bytes = {};

    public:
        constexpr std::size_t instance_count() const noexcept { return this->size(); }
    };
    
}


//Instance Data vs No Instance Data
namespace d2d::vk::impl {
    //No Instanced Data (but instanced)
    template<::d2d::impl::directly_renderable T>
    class renderable_instance_data : public renderable_fixed_data<T> {
    protected:
        std::size_t input_data_size;
    public:
        constexpr static std::size_t instance_data_size = 0;
    public:
        template<typename LoadTextureFn>
        result<std::vector<std::span<const std::byte>>> make_inputs(LoadTextureFn&&) noexcept;
        constexpr void clear_inputs() noexcept { return; }

        constexpr std::size_t input_size() const noexcept { return input_data_size; }
        consteval static VkBufferUsageFlags input_usage_flags() noexcept { return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }

    };

    //Instance Data
    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_instance_data
    class renderable_instance_data<T> : public renderable_fixed_data<T> {
        using instance_input_type = decltype(std::declval<T>().instance());
    private:
        std::vector<instance_input_type> instance_inputs;
    protected:
        std::size_t input_data_size;
    public:
        constexpr static std::size_t instance_data_size = sizeof(typename renderable_traits<T>::instance_type);
        

    public:
        template<typename LoadTextureFn>
        result<std::vector<std::span<const std::byte>>> make_inputs(LoadTextureFn&&) noexcept;
        constexpr void clear_inputs() noexcept { instance_inputs.clear(); }

        constexpr std::size_t input_size() const noexcept { return input_data_size; }
        consteval static VkBufferUsageFlags input_usage_flags() noexcept {
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | (renderable_constraints<T>::has_indices ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0);
        }
    
    };

    //No Instanced Data (and not instanced)
    template<::d2d::impl::directly_renderable T> requires (!renderable_constraints<T>::instanced)
    class renderable_instance_data<T> : public renderable_fixed_data<T> {
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

    public:
        template<typename LoadTextureFn>
        result<std::vector<std::span<const std::byte>>> make_inputs(LoadTextureFn&&) noexcept;
        constexpr void clear_inputs() noexcept { vertex_inputs.clear(); }

        constexpr std::size_t vertex_buffer_offset() const noexcept { return vertex_offset; }

        constexpr std::uint32_t vertex_count(std::size_t i) const noexcept { return vertex_counts[i]; }
        constexpr std::uint32_t first_vertex(std::size_t i) const noexcept { return vertex_firsts[i]; }

        constexpr std::size_t input_size() const noexcept { return input_data_size; }
        consteval static VkBufferUsageFlags input_usage_flags() noexcept { return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
    };
}


//Indices vs No Indices
namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    class renderable_index_data : public renderable_instance_data<T> {};

    //Non-instanced with indices
    template<::d2d::impl::directly_renderable T> requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices)
    class renderable_index_data<T> : public renderable_instance_data<T> {
        using index_input_type = decltype(std::declval<T>().indices());
    private:
        std::vector<index_input_type> index_inputs;
        std::vector<std::uint32_t> index_firsts;
        std::vector<std::uint32_t> index_counts;

    public:
        template<typename LoadTextureFn>
        result<std::vector<std::span<const std::byte>>> make_inputs(LoadTextureFn&&) noexcept;
        constexpr void clear_inputs() noexcept { index_inputs.clear(); renderable_instance_data<T>::clear_inputs(); }

        constexpr std::size_t index_buffer_offset() const noexcept { return 0; }

        constexpr std::uint32_t index_count(std::size_t i) const noexcept { return index_counts[i]; }
        constexpr std::uint32_t first_index(std::size_t i) const noexcept { return index_firsts[i]; }

        consteval static VkBufferUsageFlags input_usage_flags() noexcept { 
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT; 
        }
    };

}


//Textures vs No Textures
namespace d2d::vk::impl {
    //No Textures
    template<::d2d::impl::directly_renderable T>
    class renderable_texture_data : public renderable_index_data<T> {};

    //Textures
    template<::d2d::impl::directly_renderable T> requires (renderable_constraints<T>::has_textures)
    class renderable_texture_data<T> : public renderable_index_data<T> {
        using texture_idx_input_type = std::array<texture_idx_t, renderable_traits<T>::max_texture_count>;
    private:
        std::vector<texture_idx_input_type> texture_idx_inputs;
    protected:
        std::size_t texture_idx_offset = 0;
    private:
    public:
        template<typename LoadTextureFn>
        result<std::vector<std::span<const std::byte>>> make_inputs(LoadTextureFn&& load_texture_fn) noexcept;
        constexpr void clear_inputs() noexcept { texture_idx_inputs.clear(); renderable_index_data<T>::clear_inputs(); }

        constexpr std::size_t texture_idx_buffer_offset() const noexcept { return texture_idx_offset; }


        template<typename LoadTextureFn>
        result<std::pair<std::size_t, std::vector<std::span<const std::byte>>>> make_texture_indices(LoadTextureFn&& load_texture_fn) noexcept;
        
        template<typename SkipT, typename LoadTextureFn>
        result<void> update_texture_indices(LoadTextureFn&& load_texture_fn, renderable_allocator& allocator, buffer& data_buffer) noexcept;
    };

}


//Descriptors vs No Descriptors
namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T, std::size_t FiF>
    class renderable_descriptor_data : public renderable_texture_data<T> {
    protected:
        pipeline_layout<T> pl_layout;
    public:
        constexpr static std::size_t uniform_data_size = 0;

    public:
        result<void> create_uniform_descriptors(buffer&, std::size_t) noexcept { return {}; }
        result<void> create_texture_descriptors(texture_map&) noexcept { return {}; }
        result<void> create_pipeline_layout(std::shared_ptr<logical_device> logi_device) noexcept;
    };


    template<::d2d::impl::directly_renderable T, std::size_t FiF> requires (renderable_constraints<T>::has_uniform || renderable_constraints<T>::has_textures)
    class renderable_descriptor_data<T, FiF> : public renderable_texture_data<T> {
    private:
        constexpr static std::size_t set_binding_count = renderable_constraints<T>::has_uniform + renderable_constraints<T>::has_textures;
        constexpr static std::size_t uniform_binding      = 0;
        constexpr static std::size_t texture_binding      = renderable_constraints<T>::has_uniform;
        std::array<VkDescriptorPoolSize,         set_binding_count> pool_sizes{};
        std::array<VkDescriptorSetLayoutBinding, set_binding_count> set_layout_bindings{};
        std::array<VkDescriptorBindingFlags,     set_binding_count> set_layout_flags{};
        std::bitset<set_binding_count> valid_descriptors{};
        //TODO: move to renderable_uniform_data and renderable_image_data derived classes
        std::array<VkDescriptorBufferInfo, FiF * renderable_constraints<T>::has_uniform> uniform_buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;
    protected:
        //descriptor_set<FiF> set;
        std::array<VkDescriptorSet, FiF> sets;
        descriptor_set_layout set_layout;
        descriptor_pool pool;
        pipeline_layout<T> pl_layout;
    public:
        constexpr static std::size_t uniform_data_size = FiF * sizeof(typename renderable_traits<T>::uniform_type);

    public:
        result<void> create_uniform_descriptors(buffer& uniform_buff, std::size_t uniform_buff_offset) noexcept;
        result<void> create_texture_descriptors(texture_map& textures) noexcept;
        result<void> create_pipeline_layout(std::shared_ptr<logical_device> logi_device) noexcept;
    };
}

namespace d2d::vk {
    template<::d2d::impl::directly_renderable T, std::size_t FiF>
    class renderable_data : public impl::renderable_descriptor_data<T, FiF> {
    private:
        pipeline<T> pl;
    
    public:
        result<void> create_pipeline(std::shared_ptr<logical_device> logi_device, render_pass& window_render_pass) noexcept;

        template<std::size_t, typename>
        friend struct renderable_tuple;
    };
}


#include "Duo2D/vulkan/memory/renderable_data.inl"