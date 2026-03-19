#pragma once
#include "sirius/core/application_instance.fwd.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>

#include "sirius/core/initialize.hpp"
#include "sirius/timeline/command_traits.hpp"
#include "sirius/timeline/setup.hpp"
#include "sirius/input/category.hpp"
#include "sirius/input/code.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/map_types.hpp"
#include "sirius/input/modifier_flags.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/timeline/dedicated_command_group.hpp"
#include "sirius/vulkan/core/instance.hpp"
#include "sirius/core/render_process.hpp"
#include "sirius/core/window.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/memory/device_allocation_group.hpp"
#include "sirius/core/buffer_config_table.hpp"


namespace acma {
    template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	class application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs> :
		public window,
		public render_process<
			BufferConfigs, AssetHeapConfigs,
			timeline::command_traits<TimelineEventTs...>::group_count + timeline::impl::dedicated_command_group::num_dedicated_command_groups
		>
	{

	public:
		using command_traits_type = timeline::command_traits<TimelineEventTs...>;
		using timeline_state_type = timeline::state;
		using window_type = window;
		using render_process_type = render_process<
			BufferConfigs, AssetHeapConfigs, 
			command_traits_type::group_count + timeline::impl::dedicated_command_group::num_dedicated_command_groups
		>;

	public:
		constexpr static sl::size_t command_group_count = command_traits_type::group_count + timeline::impl::dedicated_command_group::num_dedicated_command_groups;
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t N = BufferConfigs.size();
		constexpr static sl::size_t M = AssetHeapConfigs.size();

	public:
		static result<application_instance> create(
			vk::physical_device& device, 
			bool prefer_synchronous_rendering
		) noexcept;
		static result<application_instance> create(
			vk::physical_device& device, 
			bool prefer_synchronous_rendering,
			acma::sz2u32 window_size,
			std::string_view window_title = {}
		) noexcept;

	private:
		template<bool Windowing>
		result<void> initialize(
			sl::bool_constant_type<Windowing>,
			vk::physical_device& device,
			bool prefer_synchronous_rendering
		) noexcept;

		result<void> initialize_auxiliary() noexcept;

	public:
        bool is_open() const noexcept;
		void close() noexcept;
        void poll_events() noexcept;

        std::future<result<void>> start_async_render() noexcept;
        result<void> join() const noexcept;

    public:
        result<void> render() noexcept;
	public:
		template<typename TimelineCommandT>
		result<void> execute_command(timeline_state_type& state) noexcept;
		template<typename TimelineCommandT>
		result<void> execute_command() noexcept;
	private:
		template<sl::index_t I>
		result<void> execute_command(timeline_state_type& state) noexcept;

	public:
		constexpr auto&& external_timeline_state(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._external_timeline_state); }

	private:
        std::unique_ptr<std::atomic<bool>> should_be_open;
		bool has_window;
	private:
		timeline_state_type _external_timeline_state;
	private:
		sl::tuple<typename sl::invoke_return_type_t<timeline::setup<TimelineEventTs>, render_process_type&, window_type&>::value_type...> auxiliary;

	};
}

#include "sirius/core/application_instance.inl"