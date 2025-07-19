#pragma once
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <tuple>
#include <vulkan/vulkan.h>
#include <zstring.hpp>
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/traits/buffer_traits.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/traits/renderable_properties.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"


namespace d2d::vk {
    template<std::size_t FramesInFlight, typename> struct renderable_tuple {};

    template<std::size_t FramesInFlight, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    struct renderable_tuple<FramesInFlight, std::tuple<Ts...>> {
        static_assert(sizeof...(Ts) > 0, "renderable_tuple needs at least 1 renderable type");

        template<typename T> using renderable_data_type = renderable_data<T, FramesInFlight>;
        using renderable_data_tuple_type = std::tuple<renderable_data_type<Ts>...>;
    
    public:
        //could benefit from SIMD? (just a target_clones)
        static result<renderable_tuple> create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_data_map, render_pass& window_render_pass) noexcept;
    
    protected:
        template<typename T>
        result<void> apply_changes(render_pass& window_render_pass) noexcept;
        template<typename T>
        constexpr bool const& has_changes() const noexcept;
        template<typename T>
        constexpr bool      & has_changes()       noexcept;

    public:
        template<typename T> using key_type       = typename renderable_data_type<T>::key_type;
        template<typename T> using mapped_type    = typename renderable_data_type<T>::mapped_type;
        template<typename T> using value_type     = typename renderable_data_type<T>::value_type;
        template<typename T> using iterator       = typename renderable_data_type<T>::iterator;
        template<typename T> using const_iterator = typename renderable_data_type<T>::const_iterator;


        template<typename T>
        iterator<T> end() noexcept;
        template<typename T>
        const_iterator<T> end() const noexcept;
        template<typename T>
        const_iterator<T> cend() const noexcept;

        template<typename T>
        iterator<T> begin() noexcept;
        template<typename T>
        const_iterator<T> begin() const noexcept;
        template<typename T>
        const_iterator<T> cbegin() const noexcept;

        template<typename T>
        bool empty() const noexcept;
        template<typename T>
        std::size_t size() const noexcept;


    private:
        template<typename T> constexpr const buffer& index_buffer()       const noexcept requires renderable_constraints<T>::has_indices;
        template<typename T> constexpr const buffer& uniform_buffer()     const noexcept requires renderable_constraints<T>::has_uniform;
        template<typename T> constexpr const buffer& vertex_buffer()      const noexcept requires renderable_constraints<T>::has_vertices;
        template<typename T> constexpr const buffer& instance_buffer()    const noexcept requires renderable_constraints<T>::has_instance_data;
        template<typename T> constexpr const buffer& attribute_buffer()   const noexcept requires renderable_constraints<T>::has_attributes;
        template<typename T> constexpr const buffer& texture_idx_buffer() const noexcept requires renderable_constraints<T>::has_textures;


        template<typename T> constexpr std::uint32_t index_count(std::size_t i)  const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices);
        template<typename T> constexpr std::uint32_t vertex_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices);
        template<typename T> constexpr std::size_t   instance_count()            const noexcept;

        template<typename T> constexpr std::uint32_t first_index(std::size_t i)  const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices); 
        template<typename T> constexpr std::uint32_t first_vertex(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices); 

        template<typename T> constexpr std::size_t vertex_buffer_offset()      const noexcept requires (renderable_constraints<T>::has_vertices); 
        template<typename T> constexpr std::size_t index_buffer_offset()       const noexcept requires (renderable_constraints<T>::has_indices); 
        template<typename T> constexpr std::size_t texture_idx_buffer_offset() const noexcept requires (renderable_constraints<T>::has_textures); 
        template<typename T> consteval static buffer_bytes_t static_offsets() noexcept;


        template<typename T> constexpr const pipeline<T>&                                 associated_pipeline()        const noexcept;
        template<typename T> constexpr const pipeline_layout<T>&                          associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const std::array<VkDescriptorSet, FramesInFlight>& desc_set()                   const noexcept;

        friend struct command_buffer;

    public:
        //template<typename T> constexpr const attribute_types<T>& attributes() const noexcept;
        template<typename T> constexpr std::span<typename renderable_properties<T>::uniform_type, FramesInFlight> uniform_map() const noexcept requires (renderable_constraints<T>::has_uniform);
        template<typename T> constexpr std::span<std::byte                                      , 0             > uniform_map() const noexcept requires (!renderable_constraints<T>::has_uniform);
        template<typename T> constexpr typename renderable_properties<T>::push_constant_types push_constants() const noexcept requires renderable_constraints<T>::has_push_constants;

    protected:
        template<typename T>
        constexpr renderable_data<T, FramesInFlight>      & renderable_data_of()       noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }
        template<typename T>
        constexpr renderable_data<T, FramesInFlight> const& renderable_data_of() const noexcept {
            return std::get<renderable_data<T, FramesInFlight>>(renderable_datas);
        }

        friend struct renderable_event_callbacks;

    protected:
        renderable_data_tuple_type renderable_datas;

        constexpr static std::size_t renderable_count = sizeof...(Ts);
        constexpr static std::size_t renderable_count_with_attrib = (static_cast<bool>(renderable_data<Ts, FramesInFlight>::attribute_data_size) + ...);
        //constexpr static std::size_t renderable_count_with_textures = (renderable_constraints<Ts>::has_textures + ...);
        //TODO: Find a better strategy than these manual index calculation shenanigans
        template<typename T>
        constexpr static std::size_t renderable_index = impl::buffer_type_index<T, Ts...>::value;
        template<typename T>
        constexpr static std::size_t renderable_index_with_attrib = impl::buffer_type_index<T, Ts...>::with_attrib_value;
        //template<typename T>
        //constexpr static std::size_t renderable_index_with_textures = impl::type_index<T, Ts...>::with_textures_value;


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

        std::shared_ptr<logical_device>             logi_device_ptr;
        std::shared_ptr<physical_device>            phys_device_ptr;
        std::shared_ptr<::d2d::impl::font_data_map> font_data_map_ptr;
        std::shared_ptr<command_pool>               copy_cmd_pool_ptr;
    };
}

#include "Duo2D/vulkan/memory/renderable_tuple.inl"