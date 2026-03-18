#pragma once

#include <streamline/containers/tuple.hpp>

#include "sirius/timeline/acquire_image.hpp"
#include "sirius/timeline/dispatch.hpp"
#include "sirius/timeline/initialize.hpp"
#include "sirius/timeline/submit.hpp"
#include "sirius/timeline/begin_draw_phase.hpp"
#include "sirius/timeline/end_draw_phase.hpp"
#include "sirius/timeline/draw.hpp"
#include "sirius/timeline/buffer_dependency.hpp"

#include "./generate_rects.hpp"
#include "./styled_rect.hpp"
#include "./texture_rect.hpp"

namespace acma::test {
	using basic_timeline = sl::tuple<
		acma::acquire_image,

		acma::initialize<acma::command_family::graphics>, 
		acma::begin_draw_phase,

		acma::draw<acma::test::styled_rect>,

		acma::end_draw_phase,
		acma::submit<acma::command_family::graphics>,

		acma::initialize<acma::command_family::present>,
		acma::submit<acma::command_family::present>
	>;
}

namespace acma::test {
	using novice_timeline = sl::tuple<
		acma::acquire_image,


		acma::initialize<acma::command_family::compute>,

		acma::dispatch<acma::test::generate_rects>,
		
		acma::buffer_dependency<acma::command_family::compute,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::draw_commands, acma::memory_operation::read,
			buffer_key_sequence_type<::buffer_id::draw_commands, ::buffer_id::counts>
		>,
		acma::buffer_dependency<acma::command_family::compute,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::vertex_shader, acma::memory_operation::read,
			buffer_key_sequence_type<::buffer_id::positions>
		>,
		acma::submit<acma::command_family::compute, signal_completion_at<acma::render_stage::compute_shader>>,


		acma::initialize<acma::command_family::graphics>, 
		acma::buffer_dependency<acma::command_family::graphics,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::draw_commands, acma::memory_operation::read,
			buffer_key_sequence_type<::buffer_id::draw_commands, ::buffer_id::counts>
		>,
		acma::buffer_dependency<acma::command_family::graphics,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::vertex_shader, acma::memory_operation::read,
			buffer_key_sequence_type<::buffer_id::positions>
		>,
		
		acma::begin_draw_phase,

		acma::draw<acma::test::styled_rect>,
		acma::draw<acma::test::texture_rect>,

		acma::end_draw_phase,
		acma::submit<acma::command_family::graphics, signal_completion_at<acma::render_stage::group::all_graphics>, wait_for<acma::render_stage::compute_shader>>,

		acma::initialize<acma::command_family::present>,
		acma::submit<acma::command_family::present>
	>;

}

/*
namespace acma::test {
	using intermediate_timeline = sl::tuple<
		acma::acquire_image,


		acma::initialize<acma::command_family::transfer>,
		acma::commit_transfers<buffer_key_sequence_type<1, 2>>, //buffers used by graphics

		acma::buffer_dependency<acma::command_family::transfer,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::fragment_shader, acma::memory_operation::read,
			buffer_key_sequence_type<1, 2>
		>,
		acma::submit<acma::command_family::transfer, signal_completion_at<acma::render_stage::copy>>,


		acma::initialize<acma::command_family::graphics>, 

		acma::buffer_dependency<acma::command_family::graphics,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::fragment_shader, acma::memory_operation::read,
			buffer_key_sequence_type<1, 2>
		>,


		acma::begin_draw_phase,

		acma::draw<acma::test::styled_rect>,

		acma::end_draw_phase,
		acma::submit<acma::command_family::graphics, signal_completion_at<acma::render_stage::none>, wait_for<acma::render_stage::copy>>,

		acma::initialize<acma::command_family::present>,
		acma::submit<acma::command_family::present>
	>;

}
*/

/*
namespace acma::test {
	using advanced_timeline = sl::tuple<
		acma::acquire_image,
		
		acma::initialize<acma::command_family::transfer>, 
		acma::commit_transfers<buffer_key_sequence_type<3, 4>>, //buffers used by compute

		acma::buffer_dependency<acma::command_family::transfer,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::compute_shader, acma::memory_operation::read,
			buffer_key_sequence_type<3, 4>
		>,
		acma::submit<acma::command_family::transfer, signal_completion_at<acma::render_stage::copy>>,

		acma::initialize<acma::command_family::compute>,
		acma::buffer_dependency<acma::command_family::compute,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::compute_shader, acma::memory_operation::read,
			buffer_key_sequence_type<3, 4>
		>,

		//acma::dispatch<T>
		//acma::dispatch<U>
		//acma::dispatch<V>
		
		acma::buffer_dependency<acma::command_family::compute,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::draw_commands, acma::memory_operation::read,
			buffer_key_sequence_type<0>
		>,
		acma::submit<acma::command_family::compute, signal_completion_at<acma::render_stage::compute_shader>, wait_for<acma::render_stage::copy>>,

		acma::initialize<acma::command_family::transfer>,
		acma::commit_transfers<buffer_key_sequence_type<1, 2>>, //buffers used by graphics

		acma::buffer_dependency<acma::command_family::transfer,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::fragment_shader, acma::memory_operation::read,
			buffer_key_sequence_type<1, 2>
		>,
		acma::submit<acma::command_family::transfer, signal_completion_at<acma::render_stage::copy>>,


		acma::initialize<acma::command_family::graphics>, 
		acma::buffer_dependency<acma::command_family::graphics,
			acma::render_stage::compute_shader, acma::memory_operation::write,
			acma::render_stage::draw_commands, acma::memory_operation::read,
			buffer_key_sequence_type<0>
		>,

		acma::buffer_dependency<acma::command_family::graphics,
			acma::render_stage::copy, acma::memory_operation::write,
			acma::render_stage::fragment_shader, acma::memory_operation::read,
			buffer_key_sequence_type<1, 2>
		>,
		acma::begin_draw_phase,

		acma::draw<acma::test::styled_rect>,

		acma::end_draw_phase,


		acma::submit<acma::command_family::graphics, signal_completion_at<acma::render_stage::none>, wait_for<acma::render_stage::copy | acma::render_stage::compute_shader>>,
	

		acma::initialize<acma::command_family::present>,
		acma::submit<acma::command_family::present>
	>;
}
*/