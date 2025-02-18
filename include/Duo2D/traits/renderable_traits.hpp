#pragma once
#include "Duo2D/traits/shader_input_traits.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<typename T>
    struct renderable_traits;
}


namespace d2d::impl {
    template<class T> concept Aggregate = std::is_aggregate_v<T>;
    template<class T> concept Arithmetic = std::is_arithmetic_v<T>;

    struct any_t { template<class T> operator T() {} };

    template<Aggregate T, typename... M>
    consteval std::size_t member_count(M&&... members)  {
        if constexpr (requires (T t){ T{members...}; t = {members...}; })
            return member_count<T>(std::forward<M>(members)..., any_t{});
        return sizeof...(members) - 1;
    }
    template<Arithmetic T>
    consteval std::size_t member_count()  { return 1; }
    
    template<std::size_t N, typename T>
    constexpr decltype(auto) to_tuple(size_constant<N>, T const&) { static_assert(N != N, "Too many aggregate members (not yet supported)"); }

    template<Arithmetic T> constexpr decltype(auto) to_tuple(size_constant<1>, T const& t) { return std::make_tuple(t); }

    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<0>, T const&) { return std::tuple(); }
    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<1>, T const& agg) { auto& [m1] = agg; return std::tuple(m1); }
    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<2>, T const& agg) { auto& [m1, m2] = agg; return std::tuple(m1, m2); }
    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<3>, T const& agg) { auto& [m1, m2, m3] = agg; return std::tuple(m1, m2, m3); }
    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<4>, T const& agg) { auto& [m1, m2, m3, m4] = agg; return std::tuple(m1, m2, m3, m4); }
    template<Aggregate T> constexpr decltype(auto) to_tuple(size_constant<5>, T const& agg) { auto& [m1, m2, m3, m4, m5] = agg; return std::tuple(m1, m2, m3, m4, m5); }

    template<typename T>
    struct as_tuple { using type = decltype(to_tuple(size_constant<member_count<T>()>{}, std::declval<T>())); };
    template<typename... Ts>
    struct as_tuple<std::tuple<Ts...>> { using type = std::tuple<Ts...>; };

    template<typename T> using as_tuple_t = typename as_tuple<T>::type;
};


namespace d2d::impl { 
    template<typename T> struct extract_attribute_size;
    template<typename... Ts> 
    struct extract_attribute_size<std::tuple<attribute<Ts>&...>> : 
        std::integral_constant<std::size_t, (sizeof(Ts) + ...)> {};

    template<typename T> struct extract_attribute_members;
    template<typename... Ts> 
    struct extract_attribute_members<std::tuple<attribute<Ts>&...>> { using type = decltype(std::tuple_cat(to_tuple(size_constant<member_count<Ts>()>{}, std::declval<Ts>())...));};


    template<typename T> struct extract_attribute_member_count;
    template<typename... Ts> 
    struct extract_attribute_member_count<std::tuple<attribute<Ts>&...>> : 
        std::integral_constant<std::size_t, (member_count<Ts>() + ...)> {};
}


namespace d2d {
    template<typename T>
    struct renderable : public renderable_traits<T> {
        using traits_type = renderable_traits<T>; 
        using shader_traits_type = shader_traits<T>; 
    private:
        constexpr static bool has_vertices = requires { {traits_type::vertex_count} -> std::same_as<const std::size_t&>; requires traits_type::vertex_count > 0; typename traits_type::vertex_type; };
        constexpr static bool has_attributes = requires { typename traits_type::attribute_types; };
        template<std::size_t I, typename A> using type_at_t = std::tuple_element_t<I, impl::as_tuple_t<A>>;
        constexpr static std::size_t vertex_type_count    = []() noexcept -> std::size_t {if constexpr(has_vertices)       return impl::member_count<typename traits_type::vertex_type>();                            return 0;}();
        constexpr static std::size_t instance_type_count  = []() noexcept -> std::size_t {if constexpr(shader_traits_type::instanced) return impl::member_count<typename traits_type::instance_type>();                          return 0;}();
        constexpr static std::size_t attribute_type_count = []() noexcept -> std::size_t {if constexpr(has_attributes)     return impl::extract_attribute_member_count<typename traits_type::attribute_types>::value; return 0;}();
        constexpr static std::size_t total_type_count = vertex_type_count + instance_type_count + attribute_type_count;
        constexpr static std::size_t binding_count = has_attributes + shader_traits_type::instanced + has_vertices;

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
            if constexpr(has_vertices)   ret[i++] = {i, sizeof(typename traits_type::vertex_type), VK_VERTEX_INPUT_RATE_VERTEX};
            if constexpr(T::instanced)   ret[i++] = {i, sizeof(typename traits_type::instance_type), VK_VERTEX_INPUT_RATE_INSTANCE};
            if constexpr(has_attributes) ret[i++] = {i, impl::extract_attribute_size<typename traits_type::attribute_types>::value, VK_VERTEX_INPUT_RATE_INSTANCE};//static_cast<VkVertexInputRate>(T::instanced)};
            return ret;
        }
        
        consteval static std::array<VkVertexInputAttributeDescription, total_type_count> attribute_descs() noexcept {
            std::size_t I = 0;
            std::array<VkVertexInputAttributeDescription, total_type_count> ret = {};
            std::size_t vertex_loc_size = 0, instance_loc_size = 0;
            if constexpr(has_vertices) {
                auto [vertex_inputs, vertex_size] = inputs<typename traits_type::vertex_type, vertex_type_count, 0>(0, std::make_index_sequence<vertex_type_count>{});
                vertex_loc_size = vertex_size;
                for(auto& v : vertex_inputs) ret[I++] = std::move(v);
            }
            if constexpr(T::instanced) {
                auto [instance_inputs, instance_size] = inputs<typename traits_type::instance_type, instance_type_count, has_vertices>(vertex_loc_size, std::make_index_sequence<instance_type_count>{});
                instance_loc_size = instance_size;
                for(auto& i : instance_inputs) ret[I++] = std::move(i);
            }
            if constexpr(has_attributes) {
                auto [attribute_inputs, _] = inputs<typename impl::extract_attribute_members<typename traits_type::attribute_types>::type, attribute_type_count, has_vertices + T::instanced>(vertex_loc_size + instance_loc_size, std::make_index_sequence<attribute_type_count>{});
                for(auto& a : attribute_inputs) ret[I++] = std::move(a);
            }
            return ret;
        }
    };
}


namespace d2d::impl {
    template<typename T>
    concept RenderableType = requires {
        typename T::traits_type;
        requires ShaderType<typename T::shader_type>;
        requires std::is_standard_layout_v<typename T::instance_type> || !T::instanced;
        requires (requires(T t) {{t.instance()} noexcept -> std::same_as<typename T::instance_type>;}) || !T::instanced;
        requires (requires(T t) {{t.vertices()} noexcept;}) || T::instanced;
    };

    template<typename T>
    constexpr bool has_indices_v = RenderableType<T> && requires {
        requires (requires(T t) {{t.indices()} noexcept;}) || (requires{ {T::indices()} noexcept;});
        requires std::is_same_v<typename T::index_type, std::uint16_t> || std::is_same_v<typename T::index_type, std::uint32_t>;
    };

    template<typename T>
    constexpr bool has_vertices_v = RenderableType<T> && requires {
        requires (requires(T t) {{t.vertices()} noexcept;}) || (requires{ {T::vertices()} noexcept;});
        typename T::vertex_type;
    };

    template<typename T>
    constexpr bool has_fixed_indices_v = RenderableType<T> && has_indices_v<T> && requires {
        {T::index_count} -> std::same_as<const std::size_t&>;
        requires T::index_count > 0;
    };

    template<typename T>
    constexpr bool has_fixed_vertices_v = RenderableType<T> && has_vertices_v<T> && requires {
        {T::vertex_count} -> std::same_as<const std::size_t&>;
        requires T::vertex_count > 0;
    };

    template<typename T>
    constexpr bool has_uniform_v = RenderableType<T> && requires { typename T::uniform_type; };

    template<typename T>
    constexpr bool has_push_constants_v = RenderableType<T> && requires {
        {T::push_constants()} noexcept; 
        {T::push_constant_ranges()} noexcept;
        requires std::tuple_size_v<typename T::push_constant_types> == T::push_constant_ranges().size();
    };

    template<typename T>
    constexpr bool has_attributes_v = RenderableType<T> && requires (T t) {
        {t.attributes()} noexcept; 
        std::get<0>(t.attributes());
        std::tuple_size_v<typename T::attribute_types>;
    };

    template<typename T>
    constexpr bool has_storage_v = RenderableType<T> && false; //TODO
}



namespace d2d::impl {
    template<typename T>
    struct attributes_span_type { using type = std::tuple<>; };

    template<typename T> requires has_attributes_v<T>
    struct attributes_span_type<T> { using type = std::tuple<std::span<std::byte>>; };

    template<typename... Ts>
    using attributes_tuple = decltype(std::tuple_cat(
        std::declval<typename attributes_span_type<Ts>::type>()...
    )); 
}

namespace d2d {
    template<typename... Ts>
    struct make_attribute_types { using type = std::tuple<attribute<Ts>&...>; };

    template<typename... Ts>
    using make_attribute_types_t = typename make_attribute_types<Ts...>::type;
}