#pragma once
#include <cstdint>
#include <limits>
#include <result.hpp>
#include <GLFW/glfw3.h>
#include <result/basic_error_category.hpp>
#include <frozen/unordered_map.h>
#include <string_view>
#include <system_error>
#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/queue_family.hpp"

#define __D2D_VKRESULT_TO_ERRC(vkresult) (vkresult >= 0 && vkresult <= 0b111 ? vkresult + 1000000000 : vkresult)
//(vkresult | 1000000000)
//(vkresult + (vkresult >= 0 && vkresult < 0xFF ? 1000000000 : 0))

namespace d2d::error { using code_int_t = std::int_fast32_t; }

namespace d2d::error {
    enum code : code_int_t {
        unknown,
        
        //POSIX errors
        posix_begin = 1,

        no_such_file_or_directory = static_cast<code_int_t>(std::errc::no_such_file_or_directory),
        invalid_argument          = static_cast<code_int_t>(std::errc::invalid_argument),

        posix_end = std::numeric_limits<std::uint8_t>::max(),


        //Custom errors
        no_vulkan_devices,
        device_lacks_display_format,
        device_lacks_present_mode,
        device_lacks_suitable_mem_type,
        device_not_selected,
        device_not_initialized,
        device_lacks_necessary_queue_base,
        device_lacks_necessary_queue_last = device_lacks_necessary_queue_base + queue_family::num_families - 1,
        
        window_not_found,
        window_already_exists,
        element_not_found,
        buffer_needs_changes_applied,
        invalid_image_initialization,
        descriptors_not_initialized,


        //GLFW errors
        window_system_not_initialized      = GLFW_NOT_INITIALIZED,
        invalid_window_enum_argument       = GLFW_INVALID_ENUM,
        invalid_window_argument            = GLFW_INVALID_VALUE,
        out_of_memory_for_window           = GLFW_OUT_OF_MEMORY,
        vulkan_not_supported               = GLFW_API_UNAVAILABLE,
        os_window_error                    = GLFW_PLATFORM_ERROR,
        missing_pixel_format               = GLFW_FORMAT_UNAVAILABLE,
        cannot_convert_clipboard           = GLFW_FORMAT_UNAVAILABLE,
        missing_cursor_shape               = GLFW_CURSOR_UNAVAILABLE,
        missing_window_feature             = GLFW_FEATURE_UNAVAILABLE,
        window_feature_not_yet_implemented = GLFW_FEATURE_UNIMPLEMENTED,
        missing_window_platform            = GLFW_PLATFORM_UNAVAILABLE,

        //VKResult errors
        fence_or_query_not_complete = __D2D_VKRESULT_TO_ERRC(VK_NOT_READY),
        vulkan_operation_timed_out  = __D2D_VKRESULT_TO_ERRC(VK_TIMEOUT),
        vulkan_event_signaled       = __D2D_VKRESULT_TO_ERRC(VK_EVENT_SET),
        vulkan_event_unsignaled     = __D2D_VKRESULT_TO_ERRC(VK_EVENT_RESET),
        vulkan_array_too_small      = __D2D_VKRESULT_TO_ERRC(VK_INCOMPLETE),
        swap_chain_out_of_date      = __D2D_VKRESULT_TO_ERRC(VK_SUBOPTIMAL_KHR),

        vulkan_thread_idle              = __D2D_VKRESULT_TO_ERRC(VK_THREAD_IDLE_KHR),
        vulkan_thread_done              = __D2D_VKRESULT_TO_ERRC(VK_THREAD_DONE_KHR),
        vulkan_operation_deferred       = __D2D_VKRESULT_TO_ERRC(VK_OPERATION_DEFERRED_KHR),
        vulkan_operation_not_deferred   = __D2D_VKRESULT_TO_ERRC(VK_OPERATION_NOT_DEFERRED_KHR),
        pipeline_should_be_compiled     = __D2D_VKRESULT_TO_ERRC(VK_PIPELINE_COMPILE_REQUIRED),
        missing_pipeline_cache_entry    = __D2D_VKRESULT_TO_ERRC(VK_PIPELINE_BINARY_MISSING_KHR),
        shader_incompatible_with_device = __D2D_VKRESULT_TO_ERRC(VK_INCOMPATIBLE_SHADER_BINARY_EXT),
        
        out_of_host_memory                     = __D2D_VKRESULT_TO_ERRC(VK_ERROR_OUT_OF_HOST_MEMORY),
        out_of_gpu_memory                      = __D2D_VKRESULT_TO_ERRC(VK_ERROR_OUT_OF_DEVICE_MEMORY),
        vulkan_object_initialization_failed    = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INITIALIZATION_FAILED),
        vulkan_device_lost                     = __D2D_VKRESULT_TO_ERRC(VK_ERROR_DEVICE_LOST),
        vulkan_memory_map_failed               = __D2D_VKRESULT_TO_ERRC(VK_ERROR_MEMORY_MAP_FAILED),
        missing_validation_layer               = __D2D_VKRESULT_TO_ERRC(VK_ERROR_LAYER_NOT_PRESENT),
        missing_vulkan_extension               = __D2D_VKRESULT_TO_ERRC(VK_ERROR_EXTENSION_NOT_PRESENT),
        missing_gpu_feature                    = __D2D_VKRESULT_TO_ERRC(VK_ERROR_FEATURE_NOT_PRESENT),
        vulkan_version_not_suppoted            = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INCOMPATIBLE_DRIVER),
        max_gpu_objects_reached                = __D2D_VKRESULT_TO_ERRC(VK_ERROR_TOO_MANY_OBJECTS),
        vulkan_format_not_supported            = __D2D_VKRESULT_TO_ERRC(VK_ERROR_FORMAT_NOT_SUPPORTED),
        vulkan_pool_too_fragmented             = __D2D_VKRESULT_TO_ERRC(VK_ERROR_FRAGMENTED_POOL),
        vulkan_surface_lost                    = __D2D_VKRESULT_TO_ERRC(VK_ERROR_SURFACE_LOST_KHR),
        window_in_use                          = __D2D_VKRESULT_TO_ERRC(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR),
        surface_out_of_date                    = __D2D_VKRESULT_TO_ERRC(VK_ERROR_OUT_OF_DATE_KHR),
        incompatible_display_and_image_layout  = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR),
        shader_failed_to_compile               = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INVALID_SHADER_NV),
        vulkan_pool_allocation_failed          = __D2D_VKRESULT_TO_ERRC(VK_ERROR_OUT_OF_POOL_MEMORY),
        invalid_vulkan_handle                  = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INVALID_EXTERNAL_HANDLE),
        descriptor_pool_too_fragmented         = __D2D_VKRESULT_TO_ERRC(VK_ERROR_FRAGMENTATION),
        gpu_address_not_available              = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS),
        no_exclusive_fullscreen_access         = __D2D_VKRESULT_TO_ERRC(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT),
        invalid_vulkan_usage                   = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VALIDATION_FAILED_EXT),
        no_resources_for_compression_available = __D2D_VKRESULT_TO_ERRC(VK_ERROR_COMPRESSION_EXHAUSTED_EXT),
        image_usage_not_supported              = __D2D_VKRESULT_TO_ERRC(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR),
        image_layout_not_supported             = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR),
        video_profile_operation_not_supported  = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR),
        video_profile_format_not_supported     = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR),
        video_codec_not_supported              = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR),
        video_std_header_not_supported         = __D2D_VKRESULT_TO_ERRC(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR),
        invalid_video_std_parameters           = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR),
        gpu_operation_not_permitted            = __D2D_VKRESULT_TO_ERRC(VK_ERROR_NOT_PERMITTED),
        not_enough_space_for_return_value      = __D2D_VKRESULT_TO_ERRC(VK_ERROR_NOT_ENOUGH_SPACE_KHR),
        invalid_format_modifier                = __D2D_VKRESULT_TO_ERRC(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT),
        unknown_vulkan_error                   = __D2D_VKRESULT_TO_ERRC(VK_ERROR_UNKNOWN),

        //Min and max (dependant on vulkan and subject to change)
        min_code = __D2D_VKRESULT_TO_ERRC(VK_ERROR_NOT_ENOUGH_SPACE_KHR),
        max_code = __D2D_VKRESULT_TO_ERRC(VK_PIPELINE_BINARY_MISSING_KHR),

        //Number of codes (cannot be used externally as a size)
        num_duplicate_codes = 1,
        num_unique_codes = 69,
        num_codes = num_unique_codes + num_duplicate_codes
    };
}

namespace d2d::error {
    constexpr frozen::unordered_map<code_int_t, std::string_view, code::num_unique_codes> code_descs = {
        {invalid_argument,          "Invalid argument passed to a non-graphical function"},
        {no_such_file_or_directory, "Requested file or directory does not exist"},

        {no_vulkan_devices,                                                                   "Could not find a graphics device with vulkan support"},
        {device_lacks_display_format,                                                         "The given device lacks the requested display format"},
        {device_lacks_present_mode,                                                           "The given device lacks a suitable present mode"},
        {device_lacks_suitable_mem_type,                                                      "The given device lacks the requested memory type"},
        {device_not_selected,                                                                 "A graphics device has not been selected"},
        {device_not_initialized,                                                              "The selected device has not been initialized yet"},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(queue_family::graphics), "The given device lacks a (required) graphics queue"},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(queue_family::present),  "The given device lacks a (required) present queue"},
        
        {window_system_not_initialized,      "Window system (GLFW) needs to be initialized first"},
        {invalid_window_enum_argument,       "Invalid enum argument passed to window system function"},
        {invalid_window_argument,            "Invalid argument passed to window system function"},
        {out_of_memory_for_window,           "A memory allocation for the window system failed"},
        {vulkan_not_supported,               "Vulkan was not found on the system; It's either not supported or not installed"},
        {os_window_error,                    "A platform-specific error occured within the window system"},
        {missing_pixel_format,               "Either the requested display pixel format is not supported (if specified during window creation), or the clipboard contents could not be converted to the requested format (if specified during clipboard querying)"},
        {missing_cursor_shape,               "The requested standard cursor shape is not available"},
        {missing_window_feature,             "The requested window feature is not supported by the platform"},
        {window_feature_not_yet_implemented, "The requested window feature has not been implemented in the window system yet"},
        {missing_window_platform,            "The requested window platform was not found, or, if none was specifically requested, no supported platforms were found"},

        {fence_or_query_not_complete, "The vulkan fence or query has not yet completed"},
        {vulkan_operation_timed_out,  "The vulkan-specific wait operation did not complete in the specified time"},
        {vulkan_event_signaled,       "The vulkan event was signaled"},
        {vulkan_event_unsignaled,     "The vulkan event was unsignaled"},
        {vulkan_array_too_small,      "The given return array was too small for the vulkan-specific result"},
        {swap_chain_out_of_date,      "The swap chain is out of date - it should be re-created"},

        {vulkan_thread_idle,              "The given deferred vulkan operation is not complete, but there is currently no work for the current thread"},
        {vulkan_thread_done,              "The given deferred vulkan operation is not complete, but there is no work remaining"},
        {vulkan_operation_deferred,       "For the given deferred vulkan operation that was requested, at least some of the work was deferred"},
        {vulkan_operation_not_deferred,   "For the given deferred vulkan operation that was requested, none of the work was deferred"},
        {pipeline_should_be_compiled,     "The given pipeline should be compiled, although it wasn't requested to be"},
        {missing_pipeline_cache_entry,    "The given pipleine requested to be created using an internal cache entry which does not exist"},
        {shader_incompatible_with_device, "The given shader binary is not compatible with the given device"},
        
        {out_of_host_memory,                     "A vulkan-specific host memory allocation failed - most likely because the host is out of memory"},
        {out_of_gpu_memory,                      "A vulkan-specific GPU memory allocation failed - most likely because the GPU is out of memory"},
        {vulkan_object_initialization_failed,    "Initialization of the given vulkan-specific object failed due to a vendor/implementation specific reason"},
        {vulkan_device_lost,                     "The given logical or physical vulkan device has been lost"},
        {vulkan_memory_map_failed,               "A vulkan-specific memory mapping failed"},
        {missing_validation_layer,               "Could not find/load the requested validation layer"},
        {missing_vulkan_extension,               "The requested vulkan extension is not supported by the given device"},
        {missing_gpu_feature,                    "The requested GPU feature is not supported the given device"},
        {vulkan_version_not_suppoted,            "The requested vulkan API version is not supported by the graphics driver"},
        {max_gpu_objects_reached,                "Cannot create the given object - too many vulkan objects have already been created on the given device"},
        {vulkan_format_not_supported,            "The requested vulkan format is not supported by the given device"},
        {vulkan_pool_too_fragmented,             "A vulkan pool allocation failed because the pool's memory is too fragmented (no attempt to allocate host or device memory was made)"},
        {vulkan_surface_lost,                    "The vulkan surface was lost"},
        {window_in_use,                          "The requested window is already in use by a graphics API and cannot be used again"},
        {surface_out_of_date,                    "The surface is out of date - it needs to be re-queried and have its swap chain re-created"},
        {incompatible_display_and_image_layout,  "The swap chain is either incompatible with or using a different image layout than the display"},
        {shader_failed_to_compile,               "At least one shader failed to compile or link"},
        {vulkan_pool_allocation_failed,          "A vulkan pool allocation failed for an unknown reason"},
        {invalid_vulkan_handle,                  "The given external handle is not a valid handle for the given vulkan handle type"},
        {descriptor_pool_too_fragmented,         "The requested descriptor pool could not be created due to fragmentation problems"},
        {gpu_address_not_available,              "A vulkan-specific memory allocation failed because the requested GPU address is not available"},
        {no_exclusive_fullscreen_access,         "An operation failed on the given swap chain because it did not have exclusive fullscreen access"},
        {invalid_vulkan_usage,                   "Invalid usage of a vulkan command"},
        {no_resources_for_compression_available, "Image creation failed: No resources could be allocated for required image compression"},
        {image_usage_not_supported,              "The given image usage flags are not supported"},
        {image_layout_not_supported,             "The given video/image layout is not supported"},
        {video_profile_operation_not_supported,  "The given video profile operation is not supported"},
        {video_profile_format_not_supported,     "The format parameters of the given video profile is not supported"},
        {video_codec_not_supported,              "The codec parameters of the given video profile are not supported"},
        {video_std_header_not_supported,         "The specified video STD header version is not supported"},
        {invalid_video_std_parameters,           "The specified video STD parameters are not valid"},
        {gpu_operation_not_permitted,            "The specified priority operation is not permitted by the graphics driver: Insufficient privileges"},
        {not_enough_space_for_return_value,      "The vulkan operation does not have enough space in its return value for all the required data"},
        {invalid_format_modifier,                "Invalid DRM format modifier(s) specified for plane layout"},
        {unknown_vulkan_error,                   "An unknown vulkan-related error occured"},
    };
}

OL_RESULT_DECLARE_AS_ERROR_CODE(d2d::error, code, &(ol::error_category_msg_map<decltype(code_descs), code_descs>), nullptr, duo2d)


namespace d2d {
    using errc = error::code;

    template<typename T>
    using result = ol::result<T, errc>;
}



#define __D2D_VULKAN_VERIFY(fn) if(VkResult r = fn) [[unlikely]] return static_cast<d2d::errc>(__D2D_VKRESULT_TO_ERRC(r));


namespace d2d::error {
    inline std::string_view& last_glfw_desc() {
        static std::string_view last_glfw_str;
        return last_glfw_str;
    }
}

#define __D2D_GLFW_POP_ERR(desc) int code = glfwGetError(desc) 
#define __D2D_GLFW_STORE_ERR(desc) d2d::error::last_glfw_desc() = desc
#define __D2D_GLFW_VERIFY(cond) \
if(const char* desc; !cond) { \
    __D2D_GLFW_POP_ERR(&desc); \
    __D2D_GLFW_STORE_ERR(std::string_view(desc)); \
    return static_cast<error::code>(code); \
}