#pragma once
#include "Duo2D/core/application_instance.fwd.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>

#include "Duo2D/timeline/command_traits.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/map_types.hpp"
#include "Duo2D/input/modifier_flags.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/timeline/dedicated_command_group.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/core/window.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/device_allocation_group.hpp"
#include "Duo2D/core/buffer_config_table.hpp"


namespace d2d {
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
		template<bool WindowCapability>
		static result<application_instance> create(
			std::shared_ptr<vk::instance> instance,
			std::shared_ptr<vk::logical_device> logi_device, 
			std::shared_ptr<vk::physical_device> phys_device,
			std::string_view name,
			sl::bool_constant_type<WindowCapability>
		) noexcept;
	public:
		result<void> emplace_window(d2d::sz2u32 size, std::string_view title = {}) noexcept;
		result<void> initialize() noexcept;

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
        std::shared_ptr<vk::instance> inst_ptr;
		std::string_view app_name;
	private:
		timeline_state_type _external_timeline_state;
	private:
		sl::tuple<typename sl::invoke_return_type_t<timeline::setup<TimelineEventTs>, render_process_type&, window_type&>::value_type...> auxiliary;

	};
}

#include "Duo2D/core/application_instance.inl"