#pragma once
#include <Duo2D/arith/point.hpp>
#include <Duo2D/core/buffer_config_table.hpp>
#include <Duo2D/core/buffer_config.hpp>

#include "./push_constants.hpp"


namespace buffer_id {
enum : d2d::buffer_key_t {
	draw_commands,
	single_instance_draw_command,
	counts,
	dispatch_commands,

	rectangle_indices,
	offset,
	positions,

	staging,
	texture_staging,
	compute_buffer_addresses,
	draw_constants,
	compute_constants,


	num_buffer_ids
};
}

constexpr d2d::buffer_config_table<buffer_id::num_buffer_ids> buffer_configs{{{
	{buffer_id::dispatch_commands, {d2d::memory_policy::shared, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::dispatch_commands, 0, sizeof(d2d::dispatch_command_t)}},
	{buffer_id::counts, {d2d::memory_policy::shared, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::draw_count | d2d::buffer_usage_policy::generic, 0, sizeof(d2d::draw_count_t) + sizeof(sl::uint32_t)}},
	{buffer_id::draw_commands, {d2d::memory_policy::shared, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::draw_commands, 0, 256 * sizeof(d2d::indexed_draw_command_t)}},
	{buffer_id::single_instance_draw_command, {d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::draw_commands, 0, sizeof(d2d::indexed_draw_command_t)}},


	{buffer_id::rectangle_indices, {d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::index, 0, sizeof(std::uint16_t)}},
	{buffer_id::offset, {d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::uniform, d2d::shader_stage::compute, sizeof(std::uint16_t)}},
	{buffer_id::positions, {d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::buffer_usage_policy::generic, 0, sizeof(d2d::pt2u32) * 3}},

	{buffer_id::staging, {d2d::memory_policy::cpu_local_cpu_write, d2d::coupling_policy::decoupled, d2d::buffer_usage_policy::generic, 0, sizeof(std::uint16_t)}},
	{buffer_id::texture_staging, {d2d::memory_policy::cpu_local_cpu_write, d2d::coupling_policy::decoupled, d2d::buffer_usage_policy::texture_data, 0}},
	{buffer_id::compute_buffer_addresses, {d2d::memory_policy::shared, d2d::coupling_policy::decoupled, d2d::buffer_usage_policy::generic, 0, 3 * sizeof(d2d::gpu_address_t)}}, //uniform
	{buffer_id::draw_constants, {d2d::memory_policy::push_constant, d2d::coupling_policy::decoupled, d2d::buffer_usage_policy::push_constant, d2d::shader_stage::all_graphics, sizeof(draw_constants)}},
	{buffer_id::compute_constants, {d2d::memory_policy::push_constant, d2d::coupling_policy::decoupled, d2d::buffer_usage_policy::push_constant, d2d::shader_stage::compute, sizeof(compute_constants)}},
	
}}};