#pragma once
#include <streamline/numeric/int.hpp>


namespace d2d {
	using command_family_t = sl::uint_fast8_t;

    namespace command_family {
    enum : command_family_t { 
        graphics,
		compute,
		transfer,

		present,
        
        num_families,
		num_distinct_families = num_families - 1,

		none = static_cast<command_family_t>(sl::npos)
    };
    }
}