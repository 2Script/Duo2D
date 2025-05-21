#pragma once
#include <cstdint>
#include <limits>
#include <result.hpp>
#include <GLFW/glfw3.h>
#include <result/basic_error_category.hpp>
#include <type_traits>

#include "Duo2D/vulkan/device/queue_family.hpp"


namespace d2d::error {
    enum code : std::int_fast16_t {
        __glfw_mask = 0xFFFF,

        //GLFW errors
        window_system_not_initialized = GLFW_NOT_INITIALIZED    & __glfw_mask, //TODO map to system_error
        invalid_argument              = GLFW_INVALID_VALUE      & __glfw_mask,
        os_window_error               = GLFW_PLATFORM_ERROR     & __glfw_mask,
        missing_pixel_format          = GLFW_FORMAT_UNAVAILABLE & __glfw_mask,
        cannot_convert_clipboard      = GLFW_FORMAT_UNAVAILABLE & __glfw_mask,
        __glfw_end = GLFW_NO_WINDOW_CONTEXT & __glfw_mask,

        //Vulkan errors
        vulkan_not_supported = GLFW_API_UNAVAILABLE & __glfw_mask,
        no_vulkan_devices    = __glfw_end + 1,
        missing_validation_layer,

        device_lacks_necessary_queue_base,
        device_lacks_necessary_queue_last = device_lacks_necessary_queue_base + queue_family::num_families - 1,

        device_lacks_display_format,
        device_lacks_present_mode,
        device_lacks_suitable_mem_type,

        //Internal application errors (programmer's fault)
        device_not_selected,
        device_not_initialized,
        window_not_found,
        window_already_exists,
        element_not_found,
        buffer_needs_changes_applied,
        invalid_buffer_type,
        descriptors_not_initialized,

        unknown,
        num_codes,
    };
}

namespace d2d::error {
    constexpr std::string_view code_desc[] = {
        "Window failed to initialize",
        "[Internal Error] Window system (GLFW) is not initialized"
    };
}

OL_RESULT_DECLARE_AS_ERROR_CODE(d2d::error, code, &ol::error_category_array<code_desc>, nullptr, duo2d)


namespace d2d {
    using errc = error::code;

    template<typename T>
    using result = ol::result<T, errc>;
}



#define __D2D_VULKAN_VERIFY(fn) if(VkResult r = fn) return static_cast<d2d::errc>(r);


namespace d2d::error {
    inline std::string& last_glfw_desc() {
        static std::string last_glfw_str;
        return last_glfw_str;
    }
}

#define __D2D_GLFW_POP_ERR(desc) int code = glfwGetError(desc) 
#define __D2D_GLFW_ERR static_cast<error::code>(code & error::__glfw_mask)
#define __D2D_GLFW_STORE_ERR(desc) d2d::error::last_glfw_desc() = desc
#define __D2D_GLFW_VERIFY(cond) \
if(const char* desc; !cond) { \
    __D2D_GLFW_POP_ERR(&desc); \
    __D2D_GLFW_STORE_ERR(std::string(desc)); \
    return __D2D_GLFW_ERR; \
}