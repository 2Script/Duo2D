#pragma once
#include "Duo2D/core/application.hpp"

#include <atomic>
#include <memory>
#include <ratio>
#include <utility>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"

namespace d2d {
    template<typename InstanceT, bool WindowCapability>
    result<application<InstanceT, WindowCapability>> application<InstanceT, WindowCapability>::create(std::string_view name, version app_version) noexcept {
        application ret{};
        ret.name = name;

        // Set application info
        VkApplicationInfo app_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = name.data(),
            .applicationVersion = VK_MAKE_VERSION(app_version.major(), app_version.minor(), app_version.patch()),
			.pEngineName = "Duo2D",
            .engineVersion = VK_MAKE_VERSION(engine_version.major(), engine_version.minor(), engine_version.patch()),
            .apiVersion = VK_API_VERSION_1_3
        };


		std::span<char const* const> instance_extension_names{};
		if constexpr(WindowCapability) {
			//Initialize GLFW
			ret.glfw_init_ptr = std::make_unique<impl::instance_tracker<void, glfwTerminate>>();
			RESULT_VERIFY(ret.glfw_init_ptr->initialize(impl::window_system_count(), []() noexcept -> result<void> {
			    if(int code; !glfwInit()) {
			        if((code = glfwGetError(nullptr)))
			            return static_cast<errc>(code | 0x00010000);
			        return errc::os_window_error;
			    }
			    return {};
			}));

			//Get needed extensions
        	sl::uint32_t glfw_ext_cnt = 0;
        	char const* const* glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
        	__D2D_GLFW_VERIFY(glfw_exts);
			instance_extension_names = {glfw_exts, glfw_ext_cnt};

			// Check for vulkan support
			if(!glfwVulkanSupported()) 
			    return error::vulkan_not_supported;
		}


        // Create instance
        vk::instance i;
        RESULT_TRY_MOVE(i, make<vk::instance>(app_info, instance_extension_names));
        ret.instance_ptr = std::make_shared<vk::instance>(std::move(i));

		//Create physical device list
		{
		std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(*ret.instance_ptr, &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::vector<VkPhysicalDevice> devices(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(*ret.instance_ptr, &device_count, devices.data()));


        for(VkPhysicalDevice device_handle : devices) {
            result<vk::physical_device> d = make<vk::physical_device>(device_handle, ret.instance_ptr, WindowCapability);
            if(!d.has_value()) return d.error();
            ret._devices.insert(*std::move(d));
        }
		}
		
		// Create pointers
        ret.phys_device_ptr = std::make_shared_for_overwrite<vk::physical_device>();
        ret.logi_device_ptr = std::make_shared<vk::logical_device>();

		ret.should_be_open = std::make_unique<std::atomic<bool>>(true);

        return ret;
    }
}


namespace d2d {
    template<typename InstanceT, bool WindowCapability>
    result<void> application<InstanceT, WindowCapability>::initialize_device() noexcept {
        //Create logical device
        vk::logical_device l;
        RESULT_TRY_MOVE(l, make<vk::logical_device>(phys_device_ptr, WindowCapability));
        logi_device_ptr = std::make_shared<vk::logical_device>(std::move(l));

        return {};
    }
}


namespace d2d {
    template<typename InstanceT, bool WindowCapability>
    result<InstanceT*> application<InstanceT, WindowCapability>::emplace_back() noexcept {
        if(!logi_device_ptr)
            return error::device_not_initialized;

        RESULT_VERIFY_UNSCOPED((make<InstanceT>(
			instance_ptr,
			logi_device_ptr,
			phys_device_ptr,
			name,
			sl::bool_constant<WindowCapability>
		)), instance);
        _instances.emplace_back(std::make_unique<InstanceT>(*std::move(instance)));
		return _instances.back().get();
    }
}


namespace d2d {
    template<typename InstanceT, bool WindowCapability>
    bool application<InstanceT, WindowCapability>::is_open() const noexcept {
        return should_be_open->load(std::memory_order_relaxed);
    }

    template<typename InstanceT, bool WindowCapability>
    void application<InstanceT, WindowCapability>::close() noexcept {
        should_be_open->store(false, std::memory_order_relaxed);
    }


    template<typename InstanceT, bool WindowCapability>
    void application<InstanceT, WindowCapability>::poll_events() noexcept requires (WindowCapability) {
		if(!glfw_init_ptr) return;
		
        glfwPollEvents();

        for (auto& inst : _instances) {
			if(!inst->window_handle) continue;
            if(!glfwWindowShouldClose(inst->window_handle.get()))
                return;
		}
        close();
    }

    template<typename InstanceT, bool WindowCapability>
    result<void> application<InstanceT, WindowCapability>::render() noexcept {
        for (auto& inst : _instances) 
            if(auto r = inst->render(); !r.has_value()) [[unlikely]]
                return r.error();
        return {};
    }


    template<typename InstanceT, bool WindowCapability>
    std::future<result<void>> application<InstanceT, WindowCapability>::start_async_render() noexcept {
        return std::async([](d2d::application<InstanceT, WindowCapability>& a) -> d2d::result<void> {
            while(a.is_open())
				for (auto& inst : a._instances) 
                	if(auto r = inst->render(); !r.has_value()) [[unlikely]]
                    	return r.error();
            return {};
        }, std::ref(*this));
    }

    template<typename InstanceT, bool WindowCapability>
    result<void> application<InstanceT, WindowCapability>::join() const noexcept {
        __D2D_VULKAN_VERIFY(vkDeviceWaitIdle(*logi_device_ptr));
        return {};
    }
}

//namespace d2d {
//    template<typename InstanceT, bool WindowCapability>
//    std::atomic_int64_t application<InstanceT, WindowCapability>::instance_count{};
//}