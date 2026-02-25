#pragma once
#include <Duo2D/arith/point.hpp>
#include <Duo2D/core/resource_table.hpp>
#include <Duo2D/core/resource_config.hpp>

#include "./push_constants.hpp"


namespace resource_id {
enum : d2d::resource_key_t {
	draw_commands,
	draw_count,
	dispatch_commands,

	rectangle_limit,
	rectangle_indices,
	positions,

	staging,
	compute_buffer_addresses,
	draw_constants,
	compute_constants,


	num_resource_ids
};
}

constexpr d2d::resource_table<resource_id::num_resource_ids> resource_configs{{{
	{resource_id::dispatch_commands, {d2d::memory_policy::shared, d2d::buffering_policy::single, d2d::usage_policy::dispatch_commands, 0, sizeof(d2d::dispatch_command_t)}},
	{resource_id::draw_count, {d2d::memory_policy::shared, d2d::buffering_policy::single, d2d::usage_policy::draw_count, 0, sizeof(d2d::draw_count_t)}},
	{resource_id::draw_commands, {d2d::memory_policy::shared, d2d::buffering_policy::single, d2d::usage_policy::draw_commands, 0, sizeof(d2d::indexed_draw_command_t)}},

	{resource_id::rectangle_limit, {d2d::memory_policy::gpu_local, d2d::buffering_policy::single, d2d::usage_policy::generic, 0, sizeof(d2d::vec4<sl::uint32_t>)}},
	{resource_id::rectangle_indices, {d2d::memory_policy::gpu_local, d2d::buffering_policy::single, d2d::usage_policy::index, 0, sizeof(std::uint16_t)}},
	{resource_id::positions, {d2d::memory_policy::gpu_local, d2d::buffering_policy::single, d2d::usage_policy::generic, 0, sizeof(d2d::pt2u32) * 3}},

	{resource_id::staging, {d2d::memory_policy::cpu_local, d2d::buffering_policy::multi, d2d::usage_policy::generic, 0, sizeof(std::uint16_t)}},
	{resource_id::compute_buffer_addresses, {d2d::memory_policy::shared, d2d::buffering_policy::multi, d2d::usage_policy::generic, 0, 4 * sizeof(d2d::gpu_address_t)}}, //uniform
	{resource_id::draw_constants, {d2d::memory_policy::push_constant, d2d::buffering_policy::multi, d2d::usage_policy::push_constant, d2d::shader_stage::all_graphics, sizeof(draw_constants)}},
	{resource_id::compute_constants, {d2d::memory_policy::push_constant, d2d::buffering_policy::multi, d2d::usage_policy::push_constant, d2d::shader_stage::compute, sizeof(compute_constants)}},
	
}}};