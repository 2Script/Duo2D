#pragma once
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/core/window.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	template<typename T>
	struct draw : timeline::event {
		constexpr static command_family_t family = command_family::graphics;
	};
}


namespace d2d::timeline {
	template<typename T>
	struct setup<draw<T>> {
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		result<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			window& win
		) const noexcept {
			return make<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>(
				proc.logical_device_ptr(), 
				std::span<const VkFormat, 1>{&win.swap_chain().format().pixel_format.id, 1}, 
				win.depth_image().format()
			);
		};
	};
}

namespace d2d::timeline {
	template<typename T>
	struct command<draw<T>> {
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc, 
			window&, 
			timeline::state&, 
			vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>& pipeline, 
			sl::index_constant_type<CommandGroupIdx>
		) const noexcept {
			vk::command_buffer const& graphics_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];

			graphics_buffer.bind_pipeline(pipeline);

			//For each buffer declared by type T, bind the buffer (if applicable)
			[&graphics_buffer, &proc, &pipeline]<buffer_key_t... Is>(buffer_key_sequence_type<Is...>){
				((graphics_buffer.template bind_buffer<T>(proc[sl::constant<buffer_key_t, Is>], pipeline.layout())), ...);
			}(T::buffers);

			graphics_buffer.template draw<T>(proc); 

			return {};
		};
	};
}