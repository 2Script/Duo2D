#pragma once

#include "sirius/core/coupling_policy.hpp"
#include "sirius/core/memory_policy.hpp"
#include "sirius/core/shader_stage.hpp"


namespace acma {
	struct asset_heap_config {
		memory_policy_t image_memory;
		coupling_policy_t coupling;
		shader_stage_flags_t stages;
	};
}