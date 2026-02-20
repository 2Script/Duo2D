#pragma once
#include <cstddef>
#include <streamline/containers/lookup_table.hpp>
#include <streamline/universal/make_deduced.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "Duo2D/core/resource_config.hpp"
#include "Duo2D/core/resource_id.hpp"


namespace d2d {
    using resource_key_t = std::uint64_t;

	template<resource_key_t... Ks>
	using resource_key_sequence_type = sl::integer_sequence_type<resource_key_t, Ks...>;
	template<resource_key_t K>
	using resource_key_constant_type = sl::constant_type<resource_key_t, K>;

    template<std::size_t N>
    using resource_table = sl::lookup_table<N, resource_key_t, resource_config>;


}

namespace d2d::impl {
	template<typename>
	struct is_resource_table : sl::false_constant_type {};

	template<sl::size_t N>
	struct is_resource_table<resource_table<N>> : sl::true_constant_type {};

	
    template<typename T>
	constexpr bool is_resource_table_v = is_resource_table<T>::value;
}

namespace d2d::vk::impl {
    constexpr resource_table<resource_ids::count> resource_table_defaults{{{
        {{resource_ids::textures        }, {memory_policy::gpu_local,     buffering_policy::single, usage_policy::sampled_image, shader_stage::fragment}},
        {{resource_ids::texture_indicies}, {memory_policy::gpu_local,     buffering_policy::single, usage_policy::generic,       shader_stage::fragment, sizeof(sl::uint16_t)}},
        {{resource_ids::buffer_addresses}, {memory_policy::push_constant, buffering_policy::multi,  usage_policy::push_constant, shader_stage::compute | shader_stage::all_graphics, sizeof(std::uint64_t)}},
    }}};
}

using filter_index = sl::filtered_sequence_t<sl::index_sequence_of_length_type<d2d::vk::impl::resource_table_defaults.size()>, []<sl::index_t I>(sl::index_constant_type<I>){
    return sl::get<sl::second_constant>(*std::next(d2d::vk::impl::resource_table_defaults.begin(), I)).memory == d2d::memory_policy::gpu_local;
}>;
constexpr auto d2d_y = sl::universal::make_deduced<sl::generic_lookup_table>(d2d::vk::impl::resource_table_defaults, sl::functor::subscript<0>{}, sl::functor::subscript<1>{}, filter_index{});


constexpr auto d2d_c = sl::get<sl::second_constant>(*std::next(d2d::vk::impl::resource_table_defaults.begin(), 0)).usage;
constexpr auto d2d_d = sl::get<sl::second_constant>(*std::next(d2d::vk::impl::resource_table_defaults.begin(), 1)).usage;
