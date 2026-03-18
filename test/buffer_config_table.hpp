#pragma once
#include <sirius/arith/point.hpp>
#include <sirius/core/buffer_config_table.hpp>
#include <sirius/core/buffer_config.hpp>

#include "./push_constants.hpp"


namespace buffer_id {
enum : acma::buffer_key_t {
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

constexpr acma::buffer_config_table<buffer_id::num_buffer_ids> buffer_configs{{{
	{buffer_id::dispatch_commands, {acma::memory_policy::shared, acma::coupling_policy::coupled, acma::buffer_usage_policy::dispatch_commands, 0, sizeof(acma::dispatch_command_t)}},
	{buffer_id::counts, {acma::memory_policy::shared, acma::coupling_policy::coupled, acma::buffer_usage_policy::draw_count | acma::buffer_usage_policy::generic, 0, sizeof(acma::draw_count_t) + sizeof(sl::uint32_t)}},
	{buffer_id::draw_commands, {acma::memory_policy::shared, acma::coupling_policy::coupled, acma::buffer_usage_policy::draw_commands, 0, 256 * sizeof(acma::indexed_draw_command_t)}},
	{buffer_id::single_instance_draw_command, {acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::buffer_usage_policy::draw_commands, 0, sizeof(acma::indexed_draw_command_t)}},


	{buffer_id::rectangle_indices, {acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::buffer_usage_policy::index, 0, sizeof(std::uint16_t)}},
	{buffer_id::offset, {acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::buffer_usage_policy::uniform, acma::shader_stage::compute, sizeof(std::uint16_t)}},
	{buffer_id::positions, {acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::buffer_usage_policy::generic, 0, sizeof(acma::pt2u32) * 3}},

	{buffer_id::staging, {acma::memory_policy::cpu_local_cpu_write, acma::coupling_policy::decoupled, acma::buffer_usage_policy::generic, 0, sizeof(std::uint16_t)}},
	{buffer_id::texture_staging, {acma::memory_policy::cpu_local_cpu_write, acma::coupling_policy::decoupled, acma::buffer_usage_policy::texture_data, 0}},
	{buffer_id::compute_buffer_addresses, {acma::memory_policy::shared, acma::coupling_policy::decoupled, acma::buffer_usage_policy::generic, 0, 3 * sizeof(acma::gpu_address_t)}}, //uniform
	{buffer_id::draw_constants, {acma::memory_policy::push_constant, acma::coupling_policy::decoupled, acma::buffer_usage_policy::push_constant, acma::shader_stage::all_graphics, sizeof(draw_constants)}},
	{buffer_id::compute_constants, {acma::memory_policy::push_constant, acma::coupling_policy::decoupled, acma::buffer_usage_policy::push_constant, acma::shader_stage::compute, sizeof(compute_constants)}},
	
}}};