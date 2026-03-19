#pragma once
#include <memory>
#include <set>

#include "sirius/core/version.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/device/physical_device.hpp"


#ifndef SIRIUS_WINDOWING_CAPABILITIES
#define SIRIUS_WINDOWING_CAPABILITIES true
#endif

namespace acma::impl {
	constexpr bool window_capability = SIRIUS_WINDOWING_CAPABILITIES;
}


namespace acma {
	result<void> intitialize_lib(std::string_view app_name, version app_version) noexcept;
	void terminate_lib() noexcept;
}



namespace acma::impl {
	constexpr std::string& name() noexcept {
		static std::string s{};
		return s;
	}
}

namespace acma {
	inline std::vector<vk::physical_device>& devices() noexcept {
		static std::vector<vk::physical_device> s{};
		return s;
	}
}