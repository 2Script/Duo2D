#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>


namespace d2d {
	using asset_heap_key_t = sl::uint64_t;
}

namespace d2d {
	template<asset_heap_key_t... Ks>
	using asset_heap_key_sequence_type = sl::integer_sequence_type<asset_heap_key_t, Ks...>;
	template<asset_heap_key_t... Ks>
	inline constexpr asset_heap_key_sequence_type<Ks...> asset_heap_key_sequence{};

	template<asset_heap_key_t K>
	using asset_heap_key_constant_type = sl::constant_type<asset_heap_key_t, K>;
	template<asset_heap_key_t K>
	inline constexpr asset_heap_key_constant_type<K> asset_heap_key_constant{};
}
