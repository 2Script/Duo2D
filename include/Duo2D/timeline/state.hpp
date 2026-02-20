#pragma once
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/resource_config.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources>
	using draw_command_buffer_filtered_index_sequence = sl::filtered_sequence_t<sl::index_sequence_of_length_type<N>, []<sl::index_t I>(sl::index_constant_type<I>){
	    return sl::get<sl::second_constant>(*std::next(Resources.begin(), I)).usage == usage_policy::draw_commands;
	}>; 
}


namespace d2d::timeline {
	template<sl::size_t N, resource_table<N> Resources>
	struct state {
		//using draw_buffer_offsets_type = sl::lookup_table<impl::draw_command_buffer_filtered_index_sequence<N, Resources>::size(), resource_key_t, sl::size_t>;
	public:
		//draw_buffer_offsets_type draw_buffer_offsets;
		sl::uint32_t image_index;
		sl::array<command_family::num_families, sl::uint64_t> current_command_buffer_semaphore_values;
	};
}