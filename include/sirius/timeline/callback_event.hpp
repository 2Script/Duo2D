#pragma once

namespace acma::timeline {
	namespace callback_event {
	enum {
		on_frame_begin, 
		on_frame_end,

		on_swap_chain_updated,

		
		num_callback_events
	};
	}
}