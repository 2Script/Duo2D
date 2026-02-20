#pragma once
#include <cstdint>


namespace d2d {
	using command_family_t = std::uint_fast8_t;

    namespace command_family {
    enum : command_family_t { 
        graphics,
		compute,
		transfer,

		present,
        
        num_families,
		num_distinct_families = num_families - 1,
    };
    }
}