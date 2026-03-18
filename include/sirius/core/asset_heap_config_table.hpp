#pragma once
#include <cstddef>
#include <streamline/containers/lookup_table.hpp>
#include <streamline/universal/make_deduced.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "sirius/core/asset_heap_config.hpp"
#include "sirius/core/asset_heap_key_t.hpp"


namespace acma {
    template<std::size_t M>
    using asset_heap_config_table = sl::lookup_table<M, asset_heap_key_t, asset_heap_config>;
}


namespace acma::impl {
	template<typename>
	struct is_asset_heap_config_table : sl::false_constant_type {};

	template<sl::size_t M>
	struct is_asset_heap_config_table<asset_heap_config_table<M>> : sl::true_constant_type {};

	
    template<typename T>
	constexpr bool is_asset_heap_config_table_v = is_asset_heap_config_table<T>::value;
}
