#pragma once
#include <cstddef>
#include <streamline/containers/lookup_table.hpp>
#include <streamline/universal/make_deduced.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "sirius/core/buffer_config.hpp"
#include "sirius/core/buffer_key_t.hpp"


namespace acma {
    template<std::size_t N>
    using buffer_config_table = sl::lookup_table<N, buffer_key_t, buffer_config>;
}


namespace acma::impl {
	template<typename>
	struct is_buffer_config_table : sl::false_constant_type {};

	template<sl::size_t N>
	struct is_buffer_config_table<buffer_config_table<N>> : sl::true_constant_type {};

	
    template<typename T>
	constexpr bool is_buffer_config_table_v = is_buffer_config_table<T>::value;
}
