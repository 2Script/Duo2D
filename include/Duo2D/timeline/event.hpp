#pragma once

#include "Duo2D/core/command_family.hpp"


namespace d2d::timeline {
	struct event {
		constexpr static bool ends_command_group = false;
		constexpr static command_family_t family = command_family::none;
	};
}