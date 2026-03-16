#pragma once



namespace d2d {
	using texture_usage_t = bool;
}

namespace d2d {
	enum class texture_usage : texture_usage_t {
		sampled,
		storage
	};
}