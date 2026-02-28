#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>


namespace d2d {
    using buffer_key_t = sl::uint64_t;
}

namespace d2d {
	template<buffer_key_t... Ks>
	using buffer_key_sequence_type = sl::integer_sequence_type<buffer_key_t, Ks...>;
	template<buffer_key_t... Ks>
	inline constexpr buffer_key_sequence_type<Ks...> buffer_key_sequence{};

	template<buffer_key_t K>
	using buffer_key_constant_type = sl::constant_type<buffer_key_t, K>;
	template<buffer_key_t K>
	inline constexpr buffer_key_constant_type<K> buffer_key_constant{};
}