#include "sirius/core/initialize.hpp"

#include "sirius/vulkan/core/instance.hpp"
#include "sirius/core/make.hpp"


namespace acma {
	result<void> intitialize_lib(std::string_view app_name, version app_version) noexcept {
		std::span<char const* const> instance_extension_names{};
		if constexpr(impl::window_capability) {
			//Initialize GLFW
			if(int code; !glfwInit()) {
			    if((code = glfwGetError(nullptr)))
			        return static_cast<errc>(code | 0x00010000);
			    return errc::os_window_error;
			}

			//Get needed instance extensions
        	sl::uint32_t glfw_ext_cnt = 0;
        	char const* const* glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
        	__D2D_GLFW_VERIFY(glfw_exts);
			instance_extension_names = {glfw_exts, glfw_ext_cnt};

			// Check for vulkan support
			if(!glfwVulkanSupported()) 
			    return error::vulkan_not_supported;
		}


        // Set application info
        const VkApplicationInfo app_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = app_name.data(),
            .applicationVersion = VK_MAKE_VERSION(app_version.major(), app_version.minor(), app_version.patch()),
			.pEngineName = "sirius",
            .engineVersion = VK_MAKE_VERSION(engine_version.major(), engine_version.minor(), engine_version.patch()),
            .apiVersion = VK_API_VERSION_1_3
        };


		//Set initial instance creation info
		VkInstanceCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledExtensionCount = static_cast<sl::uint32_t>(instance_extension_names.size()),
            .ppEnabledExtensionNames = instance_extension_names.data(),
        };


		//Set validation layer names
		constexpr static std::array<const std::string_view, 1> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};


		//Get validation layers (if debug)
        #if defined(NDEBUG) || defined(D2D_NO_VALIDATION_LAYERS)
        create_info.enabledLayerCount = 0;
        #else
        {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

        for(auto desired_layer : validation_layers) {
            auto cmp = [=](VkLayerProperties p){return std::memcmp(p.layerName, desired_layer.data(), desired_layer.size()) == 0;};
            if(std::find_if(layers.begin(), layers.end(), cmp) == layers.end())
                return error::missing_validation_layer;
        }
        
        using c_str_array = std::array<char const* const, validation_layers.size()>;
        constexpr static auto layer_strs = []<std::size_t... Is>(std::index_sequence<Is...>){
            return c_str_array{ validation_layers[Is].data()... };
        };
        constexpr static c_str_array layer_names = layer_strs(std::make_index_sequence<validation_layers.size()>{});
        create_info.ppEnabledLayerNames = layer_names.data();
        create_info.enabledLayerCount = layer_names.size();
        }
        #endif


		//Create instance
        __D2D_VULKAN_VERIFY(vkCreateInstance(&create_info, nullptr, &vk::impl::vulkan_instance()));


        
		//Create physical device list
		{
		std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(vk::impl::vulkan_instance(), &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::unique_ptr<VkPhysicalDevice[]> device_handles = std::make_unique_for_overwrite<VkPhysicalDevice[]>(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(vk::impl::vulkan_instance(), &device_count, device_handles.get()));


		devices().reserve(device_count);
		devices().clear();
        for(sl::uint32_t i = 0; i < device_count; ++i) {
            result<vk::physical_device> d = acma::make<vk::physical_device>(device_handles[i]);
            if(!d.has_value()) return d.error();
            devices().push_back(*std::move(d));
        }

		std::sort(devices().begin(), devices().end());
		}

		impl::name() = std::string(app_name);
		return {};
	}
}

namespace acma {
	void intitialize_lib() noexcept {
		glfwTerminate();
		vkDestroyInstance(vk::impl::vulkan_instance(), nullptr);
	}
}