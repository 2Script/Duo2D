#pragma once
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/traits/aggregate_traits.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/traits/attribute_traits.hpp"
#include "Duo2D/vulkan/traits/shader_input_traits.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include <cstdint>
#include <numeric>
#include <tuple>


namespace d2d {
    template<typename T>
    struct renderable_properties : public renderable_traits<T> {
        using traits_type = renderable_traits<T>;
        using constraints_type = renderable_constraints<T>;
    private:
        //"forward declare" of constraints we need
        //We can't just use renderable_constraints<T> yet because T itself isn't a complete type yet, only renderable_traits<T> has
        constexpr static bool has_instanced_type = requires {
            typename traits_type::instance_type;
        };

        constexpr static bool has_vertex_type = requires {
            typename traits_type::vertex_type;
        };

        constexpr static bool has_attribute_types = requires {
            std::tuple_size_v<typename traits_type::attribute_types>;
        };

        constexpr static bool has_texture_count = requires {
            {traits_type::max_texture_count} -> std::same_as<const std::size_t&>;
            requires traits_type::max_texture_count > 0;
        };

    private:
        template<std::size_t I, typename A> using type_at_t = std::tuple_element_t<I, ::d2d::impl::as_tuple_t<A>>;
        constexpr static std::size_t vertex_type_count    = []() noexcept -> std::size_t {if constexpr(has_vertex_type)     return ::d2d::impl::member_count<typename traits_type::vertex_type>();                            return 0;}();
        constexpr static std::size_t instance_type_count  = []() noexcept -> std::size_t {if constexpr(has_instanced_type)  return ::d2d::impl::member_count<typename traits_type::instance_type>();                          return 0;}();
        constexpr static std::size_t attribute_type_count = []() noexcept -> std::size_t {if constexpr(has_attribute_types) return vk::impl::attribute_traits<typename traits_type::attribute_types>::total_member_count;     return 0;}();
        constexpr static std::size_t texture_type_count   = []() noexcept -> std::size_t {if constexpr(has_texture_count)   return traits_type::max_texture_count;                                                            return 0;}();
        constexpr static std::size_t total_type_count = vertex_type_count + instance_type_count + attribute_type_count + texture_type_count;
        constexpr static std::size_t binding_count = has_attribute_types + has_instanced_type + has_vertex_type + has_texture_count;


    private:
        template<typename InputT, std::uint32_t Binding, std::size_t... Is>
        consteval static std::pair<std::array<VkVertexInputAttributeDescription, sizeof...(Is)>, std::size_t> inputs(std::uint32_t location_offset, std::index_sequence<Is...>) {
            constexpr std::size_t input_count = sizeof...(Is);
            constexpr std::array<std::size_t, input_count> sizes = {std::max(sizeof(type_at_t<Is, InputT>), alignof(type_at_t<Is, InputT>))...};
            constexpr std::array<std::size_t, input_count> location_sizes = {vk::impl::shader_input_traits<type_at_t<Is, InputT>>::location_size...};
            constexpr std::size_t total_location_size = std::accumulate(location_sizes.cbegin(), location_sizes.cend(), 0);
            std::array<std::uint32_t, input_count> offsets, location_idxs;
            std::exclusive_scan(sizes.cbegin(), sizes.cend(), offsets.begin(), 0);
            std::exclusive_scan(location_sizes.cbegin(), location_sizes.cend(), location_idxs.begin(), 0);
            return {std::array<VkVertexInputAttributeDescription, input_count>{{
                {location_offset + location_idxs[Is], Binding, vk::impl::shader_input_traits<type_at_t<Is, InputT>>::format, offsets[Is]}...
            }}, total_location_size};
        }

    public:
        consteval static std::uint32_t vertex_binding()      noexcept requires (has_vertex_type)     { return 0; }
        consteval static std::uint32_t instance_binding()    noexcept requires (has_instanced_type)  { return has_vertex_type; }
        consteval static std::uint32_t attribute_binding()   noexcept requires (has_attribute_types) { return has_vertex_type + has_instanced_type; }
        consteval static std::uint32_t texture_idx_binding() noexcept requires (has_texture_count)   { return has_vertex_type + has_instanced_type + has_attribute_types; }
    
    public:
        consteval static std::array<VkVertexInputBindingDescription, binding_count> binding_descs() noexcept {
            std::array<VkVertexInputBindingDescription, binding_count> ret = {};
            std::uint32_t i = 0;
            if constexpr(has_vertex_type)     ret[i++] = {i, sizeof(typename traits_type::vertex_type),                                     VK_VERTEX_INPUT_RATE_VERTEX};
            if constexpr(has_instanced_type)  ret[i++] = {i, sizeof(typename traits_type::instance_type),                                   VK_VERTEX_INPUT_RATE_INSTANCE};
            if constexpr(has_attribute_types) ret[i++] = {i, vk::impl::attribute_traits<typename traits_type::attribute_types>::total_size, VK_VERTEX_INPUT_RATE_INSTANCE};//static_cast<VkVertexInputRate>(T::instanced)};
            if constexpr(has_texture_count)   ret[i++] = {i, sizeof(vk::texture_idx_t) * traits_type::max_texture_count,                    VK_VERTEX_INPUT_RATE_INSTANCE};
            return ret;
        }
        
        consteval static std::array<VkVertexInputAttributeDescription, total_type_count> attribute_descs() noexcept {
            std::size_t I = 0;
            std::array<VkVertexInputAttributeDescription, total_type_count> ret = {};
            std::size_t vertex_loc_size = 0, instance_loc_size = 0, attribute_loc_size = 0;
            if constexpr(has_vertex_type) {
                auto [vertex_inputs, vertex_size] = inputs<typename traits_type::vertex_type, vertex_binding()>(0, std::make_index_sequence<vertex_type_count>{});
                vertex_loc_size = vertex_size;
                for(auto& v : vertex_inputs) ret[I++] = std::move(v);
            }
            if constexpr(has_instanced_type) {
                auto [instance_inputs, instance_size] = inputs<typename traits_type::instance_type, instance_binding()>(vertex_loc_size, std::make_index_sequence<instance_type_count>{});
                instance_loc_size = instance_size;
                for(auto& i : instance_inputs) ret[I++] = std::move(i);
            }
            if constexpr(has_attribute_types) {
                auto [attribute_inputs, attribute_size] = inputs<typename vk::impl::attribute_traits<typename traits_type::attribute_types>::tuple_type, attribute_binding()>(vertex_loc_size + instance_loc_size, std::make_index_sequence<attribute_type_count>{});
                attribute_loc_size = attribute_size;
                for(auto& a : attribute_inputs) ret[I++] = std::move(a);
            }
            if constexpr(has_texture_count) {
                auto [tex_idx_inputs, _] = inputs<std::array<vk::texture_idx_t, traits_type::max_texture_count>, texture_idx_binding()>(vertex_loc_size + instance_loc_size + attribute_loc_size, std::make_index_sequence<traits_type::max_texture_count>{});
                for(auto& t : tex_idx_inputs) ret[I++] = std::move(t);
            }
            return ret;
        }
    };
}

namespace d2d {
    template<impl::renderable_container_like T>
    struct renderable_properties<T> {};

    template<>
    struct renderable_properties<font> {};
}