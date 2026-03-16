#pragma once

#include "Duo2D/core/coupling_policy.hpp"
#include "Duo2D/core/memory_policy.hpp"
#include "Duo2D/core/shader_stage.hpp"


namespace d2d {
	struct asset_heap_config {
		memory_policy_t image_memory;
		coupling_policy_t coupling;
		shader_stage_flags_t stages;
	};
}