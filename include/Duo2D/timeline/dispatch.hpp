#pragma once
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	template<typename T>
	struct dispatch : timeline::event {
		constexpr static command_family_t family = command_family::compute;
	};
}


namespace d2d::timeline {
	template<typename T>
	struct setup<dispatch<T>> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
		result<vk::pipeline<vk::bind_point::compute, T, N, Resources>> operator()(render_process<N, Resources, CommandGroupCount> const& proc) const noexcept {
			return make<vk::pipeline<vk::bind_point::compute, T, N, Resources>>(proc.logical_device_ptr());
		};
	};
}

namespace d2d::timeline {
	template<typename T>
	struct command<dispatch<T>> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(render_process<N, Resources, CommandGroupCount> const& proc, timeline::state<N, Resources, CommandGroupCount>&, vk::pipeline<vk::bind_point::compute, T, N, Resources>& pipeline, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			vk::command_buffer<N> const& compute_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];

			compute_buffer.bind_pipeline(pipeline);

			sl::size_t bind_index = 0;
			//For each resource declared by type T, bind the resource (if applicable)
			[&compute_buffer, &bind_index, &proc, &pipeline]<resource_key_t... Is>(resource_key_sequence_type<Is...>){
				((compute_buffer.template bind_buffer<T>(proc[sl::constant<resource_key_t, Is>], pipeline.layout(), bind_index)), ...);
			}(T::buffers);

			compute_buffer.template dispatch<T>(); 
			return {};
		};
	};
}