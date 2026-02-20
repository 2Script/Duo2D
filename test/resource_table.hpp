#pragma once
#include <Duo2D/arith/point.hpp>
#include <Duo2D/core/resource_table.hpp>

#include "./push_constants.hpp"


namespace resource_id {
enum : d2d::resource_key_t {
	draw_commands,

	rectangle_indices,
	positions,

	staging,
	push_constants,
};
}

constexpr d2d::resource_table<5> resource_configs{{{
	{resource_id::draw_commands, {d2d::memory_policy::shared, d2d::buffering_policy::single, d2d::usage_policy::draw_commands, 0, sizeof(VkDrawIndexedIndirectCommand)}},

	{resource_id::rectangle_indices, {d2d::memory_policy::gpu_local, d2d::buffering_policy::single, d2d::usage_policy::index, 0, sizeof(std::uint16_t)}},
	{resource_id::positions, {d2d::memory_policy::gpu_local, d2d::buffering_policy::single, d2d::usage_policy::generic, 0, sizeof(d2d::pt2u32) * 3}},

	{resource_id::staging, {d2d::memory_policy::cpu_local, d2d::buffering_policy::multi, d2d::usage_policy::generic, 0, sizeof(std::uint16_t)}},
	{resource_id::push_constants, {d2d::memory_policy::push_constant, d2d::buffering_policy::multi, d2d::usage_policy::push_constant, d2d::shader_stage::all_graphics, sizeof(push_constants)}},
	
}}};