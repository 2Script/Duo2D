#pragma once

namespace d2d::timeline::impl {
	namespace dedicated_command_group {
	enum {
		out_of_timeline_execute,
		out_of_timeline_copy,
		realloc,

		num_dedicated_command_groups
	};
	}
}