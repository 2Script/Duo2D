#pragma once
#include <Duo2D/core/asset_heap_config_table.hpp>
#include <Duo2D/core/asset_heap_config.hpp>

#include "./buffer_config_table.hpp"

namespace asset_heap_id {
enum : d2d::asset_heap_key_t {
	start = buffer_id::num_buffer_ids,

	compute,
	graphics,

	num_total_ids,
	num_asset_heap_ids = num_total_ids - start - 1,
};
}

constexpr d2d::asset_heap_config_table<asset_heap_id::num_asset_heap_ids> asset_heap_configs{{{
	{asset_heap_id::compute, d2d::asset_heap_config{d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::shader_stage::compute}},
	
	{asset_heap_id::graphics, d2d::asset_heap_config{d2d::memory_policy::gpu_local, d2d::coupling_policy::coupled, d2d::shader_stage::all_graphics}},
}}};