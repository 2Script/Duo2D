#pragma once

#include "sirius/core/command_family.hpp"


namespace acma::timeline {
	struct event {
		constexpr static bool ends_command_group = false;
		constexpr static command_family_t family = command_family::none;
	};
}