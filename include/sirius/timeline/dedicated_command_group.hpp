#pragma once

namespace acma::timeline::impl {
	namespace dedicated_command_group {
	enum {
		out_of_timeline_execute,
		out_of_timeline_copy,
		realloc,
		image_data_upload,

		num_dedicated_command_groups
	};
	}
}