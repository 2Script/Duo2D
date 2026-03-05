#pragma once
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/core/window.hpp"
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
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		result<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			window&
		) const noexcept {
			return make<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>>(proc.logical_device_ptr());
		};
	};
}

namespace d2d::timeline {
	template<typename T>
	struct command<dispatch<T>> {
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			window&,
			timeline::state&,
			vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>& pipeline,
			sl::index_constant_type<CommandGroupIdx>
		) const noexcept {
			vk::command_buffer const& compute_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];

			compute_buffer.bind_pipeline(pipeline);

			//For each buffer declared by type T, bind the buffer (if applicable)
			[&compute_buffer, &proc, &pipeline]<buffer_key_t... Ks>(buffer_key_sequence_type<Ks...>){
				((compute_buffer.template bind_buffer<T>(proc[buffer_key_constant<Ks>], pipeline.layout())), ...);
			}(T::buffers);

			compute_buffer.template dispatch<T>(proc); 
			return {};
		};
	};
}