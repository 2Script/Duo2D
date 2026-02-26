#pragma once

#include <streamline/containers/tuple.hpp>

#include "Duo2D/timeline/acquire_image.hpp"
#include "Duo2D/timeline/dispatch.hpp"
#include "Duo2D/timeline/initialize.hpp"
#include "Duo2D/timeline/submit.hpp"
#include "Duo2D/timeline/begin_draw_phase.hpp"
#include "Duo2D/timeline/end_draw_phase.hpp"
#include "Duo2D/timeline/draw.hpp"
#include "Duo2D/timeline/resource_dependency.hpp"

#include "./generate_rects.hpp"
#include "./styled_rect.hpp"

namespace d2d::test {
	using basic_timeline = sl::tuple<
		d2d::acquire_image,

		d2d::initialize<d2d::command_family::graphics>, 
		d2d::begin_draw_phase,

		d2d::draw<d2d::test::styled_rect>,

		d2d::end_draw_phase,
		d2d::submit<d2d::command_family::graphics>,

		d2d::initialize<d2d::command_family::present>,
		d2d::submit<d2d::command_family::present>
	>;
}

namespace d2d::test {
	using novice_timeline = sl::tuple<
		d2d::acquire_image,


		d2d::initialize<d2d::command_family::compute>,

		d2d::dispatch<d2d::test::generate_rects>,
		
		d2d::resource_dependency<d2d::command_family::compute,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::draw_commands, d2d::memory_operation::read,
			resource_key_sequence_type<::resource_id::draw_commands, ::resource_id::counts>
		>,
		d2d::resource_dependency<d2d::command_family::compute,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::vertex_shader, d2d::memory_operation::read,
			resource_key_sequence_type<::resource_id::positions>
		>,
		d2d::submit<d2d::command_family::compute, signal_completion_at<d2d::render_stage::compute_shader>>,


		d2d::initialize<d2d::command_family::graphics>, 
		d2d::resource_dependency<d2d::command_family::graphics,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::draw_commands, d2d::memory_operation::read,
			resource_key_sequence_type<::resource_id::draw_commands, ::resource_id::counts>
		>,
		d2d::resource_dependency<d2d::command_family::graphics,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::vertex_shader, d2d::memory_operation::read,
			resource_key_sequence_type<::resource_id::positions>
		>,
		
		d2d::begin_draw_phase,

		d2d::draw<d2d::test::styled_rect>,

		d2d::end_draw_phase,
		d2d::submit<d2d::command_family::graphics, signal_completion_at<d2d::render_stage::group::all_graphics>, wait_for<d2d::render_stage::compute_shader>>,

		d2d::initialize<d2d::command_family::present>,
		d2d::submit<d2d::command_family::present>
	>;

}

/*
namespace d2d::test {
	using intermediate_timeline = sl::tuple<
		d2d::acquire_image,


		d2d::initialize<d2d::command_family::transfer>,
		d2d::commit_transfers<resource_key_sequence_type<1, 2>>, //buffers used by graphics

		d2d::resource_dependency<d2d::command_family::transfer,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::fragment_shader, d2d::memory_operation::read,
			resource_key_sequence_type<1, 2>
		>,
		d2d::submit<d2d::command_family::transfer, signal_completion_at<d2d::render_stage::copy>>,


		d2d::initialize<d2d::command_family::graphics>, 

		d2d::resource_dependency<d2d::command_family::graphics,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::fragment_shader, d2d::memory_operation::read,
			resource_key_sequence_type<1, 2>
		>,


		d2d::begin_draw_phase,

		d2d::draw<d2d::test::styled_rect>,

		d2d::end_draw_phase,
		d2d::submit<d2d::command_family::graphics, signal_completion_at<d2d::render_stage::none>, wait_for<d2d::render_stage::copy>>,

		d2d::initialize<d2d::command_family::present>,
		d2d::submit<d2d::command_family::present>
	>;

}
*/

/*
namespace d2d::test {
	using advanced_timeline = sl::tuple<
		d2d::acquire_image,
		
		d2d::initialize<d2d::command_family::transfer>, 
		d2d::commit_transfers<resource_key_sequence_type<3, 4>>, //buffers used by compute

		d2d::resource_dependency<d2d::command_family::transfer,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::compute_shader, d2d::memory_operation::read,
			resource_key_sequence_type<3, 4>
		>,
		d2d::submit<d2d::command_family::transfer, signal_completion_at<d2d::render_stage::copy>>,

		d2d::initialize<d2d::command_family::compute>,
		d2d::resource_dependency<d2d::command_family::compute,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::compute_shader, d2d::memory_operation::read,
			resource_key_sequence_type<3, 4>
		>,

		//d2d::dispatch<T>
		//d2d::dispatch<U>
		//d2d::dispatch<V>
		
		d2d::resource_dependency<d2d::command_family::compute,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::draw_commands, d2d::memory_operation::read,
			resource_key_sequence_type<0>
		>,
		d2d::submit<d2d::command_family::compute, signal_completion_at<d2d::render_stage::compute_shader>, wait_for<d2d::render_stage::copy>>,

		d2d::initialize<d2d::command_family::transfer>,
		d2d::commit_transfers<resource_key_sequence_type<1, 2>>, //buffers used by graphics

		d2d::resource_dependency<d2d::command_family::transfer,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::fragment_shader, d2d::memory_operation::read,
			resource_key_sequence_type<1, 2>
		>,
		d2d::submit<d2d::command_family::transfer, signal_completion_at<d2d::render_stage::copy>>,


		d2d::initialize<d2d::command_family::graphics>, 
		d2d::resource_dependency<d2d::command_family::graphics,
			d2d::render_stage::compute_shader, d2d::memory_operation::write,
			d2d::render_stage::draw_commands, d2d::memory_operation::read,
			resource_key_sequence_type<0>
		>,

		d2d::resource_dependency<d2d::command_family::graphics,
			d2d::render_stage::copy, d2d::memory_operation::write,
			d2d::render_stage::fragment_shader, d2d::memory_operation::read,
			resource_key_sequence_type<1, 2>
		>,
		d2d::begin_draw_phase,

		d2d::draw<d2d::test::styled_rect>,

		d2d::end_draw_phase,


		d2d::submit<d2d::command_family::graphics, signal_completion_at<d2d::render_stage::none>, wait_for<d2d::render_stage::copy | d2d::render_stage::compute_shader>>,
	

		d2d::initialize<d2d::command_family::present>,
		d2d::submit<d2d::command_family::present>
	>;
}
*/