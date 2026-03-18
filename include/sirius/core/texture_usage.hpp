#pragma once



namespace acma {
	using texture_usage_t = bool;
}

namespace acma {
	enum class texture_usage : texture_usage_t {
		sampled,
		storage
	};
}