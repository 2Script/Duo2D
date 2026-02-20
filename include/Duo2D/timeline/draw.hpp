#pragma once
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"


namespace d2d {
	template<typename T>
	struct draw {};
}


namespace d2d::timeline {
	template<typename T>
	struct setup<draw<T>> {
		template<sl::size_t N, resource_table<N> Resources>
		result<vk::pipeline<T, N, Resources>> operator()(render_process<N, Resources> const& proc) const noexcept {
			return make<vk::pipeline<T, N, Resources>>(proc.logical_device_ptr(), std::span<const VkFormat, 1>{&proc.swap_chain().format().pixel_format.id, 1}, proc.depth_image().format());
		};
	};
}

namespace d2d::timeline {
	template<typename T>
	struct command<draw<T>> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources> const& proc, timeline::state<N, Resources>&, vk::pipeline<T, N, Resources>& pipeline) const noexcept {
			vk::command_buffer<N> const& graphics_buffer = proc.command_buffers()[proc.frame_index()][command_family::graphics];

			graphics_buffer.bind_pipeline(pipeline);

			sl::size_t bind_index = 0;
			//For each resource declared by type T, bind the resource (if possible)
			[&graphics_buffer, &bind_index, &proc, &pipeline]<resource_key_t... Is>(resource_key_sequence_type<Is...>){
				((graphics_buffer.template bind_buffer<T>(proc[sl::constant<resource_key_t, Is>], pipeline.layout(), bind_index)), ...);
			}(T::buffers);

			graphics_buffer.template draw<T>(bind_index); //dummy parameter

			return {};
		};
	};
}