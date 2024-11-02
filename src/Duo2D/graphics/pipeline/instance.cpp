#include "Duo2D/graphics/pipeline/instance.hpp"

#include <cstring>
#include <algorithm>
#include <string_view>
#include <utility>

namespace d2d {
    result<instance> instance::create(VkApplicationInfo& app_info) noexcept {
        // Get needed extensions
        uint32_t glfw_ext_cnt = 0;
        const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
        __D2D_GLFW_VERIFY(glfw_exts);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_ext_cnt;
        create_info.ppEnabledExtensionNames = glfw_exts;

        #ifdef NDEBUG
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
        
        using c_str_array = std::array<const char* const, validation_layers.size()>;
        constexpr static auto layer_strs = []<std::size_t... Is>(std::index_sequence<Is...>){
            return c_str_array{ validation_layers[Is].data()... };
        };
        constexpr static c_str_array layer_names = layer_strs(std::make_index_sequence<validation_layers.size()>{});
        create_info.ppEnabledLayerNames = layer_names.data();
        create_info.enabledLayerCount = layer_names.size();
        }
        #endif
        
        instance ret{};
        __D2D_VULKAN_VERIFY(vkCreateInstance(&create_info, nullptr, &ret.handle));
        return ret;
    }
}