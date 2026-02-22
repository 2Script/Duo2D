#pragma once
#include <streamline/containers/tuple.hpp>
#include <streamline/containers/array.hpp>
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/command_family.hpp"
#include "Duo2D/core/render_process.fwd.hpp"
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/timeline/setup.hpp"


namespace d2d::timeline::impl {
	template<typename TupleT, sl::index_t I, typename IdxSeq, typename FamilySeq>
	struct command_traits;

	template<typename Head, typename... Ts, sl::index_t I, sl::index_t... Is, command_family_t... CFs>
	struct command_traits<sl::tuple<Head, Ts...>, I, sl::index_sequence_type<Is...>, sl::integer_sequence_type<command_family_t, CFs...>> : 
		command_traits<
			sl::tuple<Ts...>, 
			I + static_cast<sl::index_t>(Head::ends_command_group),
			sl::index_sequence_type<Is..., I>,
			sl::conditional_t<
				Head::ends_command_group, 
				sl::integer_sequence_type<command_family_t, CFs..., Head::family>,
				sl::integer_sequence_type<command_family_t, CFs...>
			>
			//sl::conditional_t<
			//	!sl::traits::is_same_as_v<
			//		sl::invoke_return_type_t<setup<Head>, render_process<N, Resources, CommandGroupCount>&>,
			//		result<sl::empty_t>
			//	>,
			//	sl::tuple<AuxTs..., typename sl::invoke_return_type_t<setup<Head>, render_process<N, Resources, CommandGroupCount>&>::value_type>,
			//	sl::tuple<AuxTs...>
			//>
		> {};
	
	template<sl::index_t I, sl::index_t... Is, command_family_t... CFs>
	struct command_traits<sl::tuple<>, I, sl::index_sequence_type<Is...>, sl::integer_sequence_type<command_family_t, CFs...>> {
		constexpr static sl::array<sizeof...(Is), sl::index_t> group_indices{{Is...}};
		constexpr static sl::size_t group_count = I;
		constexpr static sl::array<group_count, command_family_t> group_families{{CFs...}};
	};
}

namespace d2d::timeline {
	template<typename... Ts>
	using command_traits = impl::command_traits<
		sl::tuple<Ts...>, 
		0, 
		sl::index_sequence_type<>, 
		sl::integer_sequence_type<command_family_t>
	>;
}