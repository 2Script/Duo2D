#pragma once
#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <string_view>
#include <set>
#include <streamline/containers/version.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "sirius/core/error.hpp"
#include "sirius/vulkan/core/instance.hpp"
#include "sirius/traits/instance_tracker.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/core/window.hpp"


namespace acma {
	using version = sl::generic::version<sl::uint16_t>;

	constexpr version engine_version{0, 0, 1}; 
}


namespace acma::impl {
	template<typename InstanceT, bool WindowCapability, typename Seq>
	class verify_instance_comatibility;


	template<typename InstanceT, bool WindowCapability, sl::index_t... Is>
	class verify_instance_comatibility<InstanceT, WindowCapability, sl::index_sequence_type<Is...>> {
		using command_traits_type = typename InstanceT::command_traits_type;
		constexpr static auto command_family_of_group = command_traits_type::group_families;
		static_assert(
			((command_family_of_group[Is] != command_family::present) && ...) || WindowCapability,
			"Render timeline contains an event which requires the present command family - "
			"You must enable window capability to use the present command family"
		);
	};
}


namespace acma {
    template<typename InstanceT, bool WindowCapability = true>
    class application : impl::verify_instance_comatibility<
		InstanceT, 
		WindowCapability, 
		sl::index_sequence_of_length_type<InstanceT::command_traits_type::group_count>
	> {
    public:
        application() noexcept = default;
        static result<application> create(std::string_view name, version app_version, bool prefer_synchronous_rendering = false) noexcept;


    public:
        constexpr std::set<vk::physical_device> const& devices() const noexcept { return _devices; }

        const vk::physical_device& selected_device() const& noexcept { return *phys_device_ptr; }
        vk::physical_device& selected_device() & noexcept { return *phys_device_ptr; }

        result<void> initialize_device() noexcept;


    public:
        //TODO: better interface for instances
		result<InstanceT*> emplace_back() noexcept;

    public:
        bool is_open() const noexcept;
		void close() noexcept;
        void poll_events() noexcept requires (WindowCapability);
        result<void> render() noexcept;
        std::future<result<void>> start_async_render() noexcept;
        result<void> join() const noexcept;

        
    private:
        std::shared_ptr<vk::instance>        instance_ptr;
        std::shared_ptr<vk::physical_device> phys_device_ptr;
        std::shared_ptr<vk::logical_device>  logi_device_ptr;
        std::string name;

        //ORDER MATTERS: glfw must be terminated after all windows have been destroyed
        std::unique_ptr<impl::instance_tracker<void, glfwTerminate>> glfw_init_ptr;
        std::vector<std::unique_ptr<InstanceT>> _instances;
		std::set<vk::physical_device> _devices;

        std::unique_ptr<std::atomic<bool>> should_be_open;
    private:
        //inline static std::atomic_int64_t instance_count;
    };
}


namespace acma::impl {
    inline std::atomic_int64_t& window_system_count() {
        static std::atomic_int64_t count{};
        return count;
    }
}


namespace acma {
    inline std::uintmax_t nanoseconds() noexcept {
        return static_cast<std::uintmax_t>(glfwGetTime() * std::nano::den);
    }
}



#include "sirius/core/application.inl"
