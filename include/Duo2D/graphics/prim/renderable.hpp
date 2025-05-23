#pragma once
#include "Duo2D/traits/aggregate_traits.hpp"
#include "Duo2D/traits/attribute_traits.hpp"
#include <cstdint>
#include <numeric>
#include <tuple>


namespace d2d {
    template<typename T>
    struct renderable_traits;

    template<typename T>
    struct renderable_constraints;
}


namespace d2d {
    template<typename T>
    struct renderable : public renderable_traits<T>, public renderable_constraints<T> {
        using constraints_type = renderable_constraints<T>;
        using traits_type = renderable_traits<T>; 
        using shader_traits_type = shader_traits<T>; 
    private:
        //"forward declare" of constraints we need
        constexpr static bool has_vertex_count = requires {
            {traits_type::vertex_count} -> std::same_as<const std::size_t&>;
            requires traits_type::vertex_count > 0;
        };

        constexpr static bool has_attribute_types = requires {
            std::tuple_size_v<typename traits_type::attribute_types>;
        };

    private:
        template<std::size_t I, typename A> using type_at_t = std::tuple_element_t<I, impl::as_tuple_t<A>>;
        constexpr static std::size_t vertex_type_count    = []() noexcept -> std::size_t {if constexpr(has_vertex_count)              return impl::member_count<typename traits_type::vertex_type>();                            return 0;}();
        constexpr static std::size_t instance_type_count  = []() noexcept -> std::size_t {if constexpr(shader_traits_type::instanced) return impl::member_count<typename traits_type::instance_type>();                          return 0;}();
        constexpr static std::size_t attribute_type_count = []() noexcept -> std::size_t {if constexpr(has_attribute_types)           return impl::attribute_traits<typename traits_type::attribute_types>::total_member_count;  return 0;}();
        constexpr static std::size_t total_type_count = vertex_type_count + instance_type_count + attribute_type_count;
        constexpr static std::size_t binding_count = has_attribute_types + shader_traits_type::instanced + has_vertex_count;


    private:
        template<typename InputT, std::size_t InputCount, std::uint32_t Binding, std::size_t... Is>
        consteval static std::pair<std::array<VkVertexInputAttributeDescription, InputCount>, std::size_t> inputs(std::uint32_t location_offset, std::index_sequence<Is...>) {
            constexpr std::array<std::size_t, InputCount> sizes = {sizeof(type_at_t<Is, InputT>)...};
            constexpr std::array<std::size_t, InputCount> location_sizes = {impl::shader_input_traits<type_at_t<Is, InputT>>::location_size...};
            constexpr std::size_t total_location_size = std::accumulate(location_sizes.cbegin(), location_sizes.cend(), 0);
            std::array<std::uint32_t, InputCount> offsets, location_idxs;
            std::exclusive_scan(sizes.cbegin(), sizes.cend(), offsets.begin(), 0);
            std::exclusive_scan(location_sizes.cbegin(), location_sizes.cend(), location_idxs.begin(), 0);
            return {std::array<VkVertexInputAttributeDescription, InputCount>{{
                {location_offset + location_idxs[Is], Binding, impl::shader_input_traits<type_at_t<Is, InputT>>::format, offsets[Is]}...
            }}, total_location_size};
        }
    
    public:
        consteval static std::array<VkVertexInputBindingDescription, binding_count> binding_descs() noexcept {
            std::array<VkVertexInputBindingDescription, binding_count> ret = {};
            std::uint32_t i = 0;
            if constexpr(has_vertex_count) ret[i++] = {i, sizeof(typename traits_type::vertex_type), VK_VERTEX_INPUT_RATE_VERTEX};
            if constexpr(T::instanced)                         ret[i++] = {i, sizeof(typename traits_type::instance_type), VK_VERTEX_INPUT_RATE_INSTANCE};
            if constexpr(has_attribute_types)     ret[i++] = {i, impl::attribute_traits<typename traits_type::attribute_types>::total_size, VK_VERTEX_INPUT_RATE_INSTANCE};//static_cast<VkVertexInputRate>(T::instanced)};
            return ret;
        }
        
        consteval static std::array<VkVertexInputAttributeDescription, total_type_count> attribute_descs() noexcept {
            std::size_t I = 0;
            std::array<VkVertexInputAttributeDescription, total_type_count> ret = {};
            std::size_t vertex_loc_size = 0, instance_loc_size = 0;
            if constexpr(has_vertex_count) {
                auto [vertex_inputs, vertex_size] = inputs<typename traits_type::vertex_type, vertex_type_count, 0>(0, std::make_index_sequence<vertex_type_count>{});
                vertex_loc_size = vertex_size;
                for(auto& v : vertex_inputs) ret[I++] = std::move(v);
            }
            if constexpr(T::instanced) {
                auto [instance_inputs, instance_size] = inputs<typename traits_type::instance_type, instance_type_count, has_vertex_count>(vertex_loc_size, std::make_index_sequence<instance_type_count>{});
                instance_loc_size = instance_size;
                for(auto& i : instance_inputs) ret[I++] = std::move(i);
            }
            if constexpr(has_attribute_types) {
                auto [attribute_inputs, _] = inputs<typename impl::attribute_traits<typename traits_type::attribute_types>::tuple_type, attribute_type_count, has_vertex_count + T::instanced>(vertex_loc_size + instance_loc_size, std::make_index_sequence<attribute_type_count>{});
                for(auto& a : attribute_inputs) ret[I++] = std::move(a);
            }
            return ret;
        }
    };
}