#pragma once
#include <cstdint>
#include <cstring>
#include <string_view>
#include <system_error>

#include <ktx.h>
#include <limits>
#include <result.hpp>
#include <GLFW/glfw3.h>
#include <result/basic_error_category.hpp>
#include <frozen/unordered_map.h>
#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/queue_family.hpp"


#define __D2D_VKRESULT_TO_ERRC(vkresult) (vkresult >= 0 && vkresult <= 0b111 ? vkresult + 1000000000 : vkresult)
#define __D2D_KTX_ERROR_TO_ERRC(ktx_error) (ktx_error + 0x100000)
#define __D2D_FTERR_TO_ERRC(fterr) (fterr | 0x00020000)


namespace d2d::error { 
    using code_int_t = std::int_fast32_t; 
}


namespace d2d::error {
    enum code : code_int_t {
        unknown,
        
        //POSIX errors
        posix_begin = 1,

        operation_not_permitted        = static_cast<code_int_t>(std::errc::operation_not_permitted),
        no_such_file_or_directory      = static_cast<code_int_t>(std::errc::no_such_file_or_directory),
        no_such_process                = static_cast<code_int_t>(std::errc::no_such_process),
        interrupted                    = static_cast<code_int_t>(std::errc::interrupted),
        io_error                       = static_cast<code_int_t>(std::errc::io_error),
        no_such_device_or_address      = static_cast<code_int_t>(std::errc::no_such_device_or_address),
        argument_list_too_long         = static_cast<code_int_t>(std::errc::argument_list_too_long),

        bad_file_descriptor            = static_cast<code_int_t>(std::errc::bad_file_descriptor),

        resource_unavailable_try_again = static_cast<code_int_t>(std::errc::resource_unavailable_try_again),
        not_enough_memory              = static_cast<code_int_t>(std::errc::not_enough_memory),
        permission_denied              = static_cast<code_int_t>(std::errc::permission_denied),
        bad_address                    = static_cast<code_int_t>(std::errc::bad_address),

        device_or_resource_busy        = static_cast<code_int_t>(std::errc::device_or_resource_busy),

        no_such_device                 = static_cast<code_int_t>(std::errc::no_such_device),
        not_a_directory                = static_cast<code_int_t>(std::errc::not_a_directory),
        is_a_directory                 = static_cast<code_int_t>(std::errc::is_a_directory),
        invalid_argument               = static_cast<code_int_t>(std::errc::invalid_argument),
        too_many_files_open_in_system  = static_cast<code_int_t>(std::errc::too_many_files_open_in_system),
        too_many_files_open            = static_cast<code_int_t>(std::errc::too_many_files_open),

        file_too_large                 = static_cast<code_int_t>(std::errc::file_too_large),

        invalid_seek                   = static_cast<code_int_t>(std::errc::invalid_seek),

        broken_pipe                    = static_cast<code_int_t>(std::errc::broken_pipe),

        function_not_supported         = static_cast<code_int_t>(std::errc::function_not_supported),

        invalid_font_file_format       = EBFONT,

        value_too_large                = static_cast<code_int_t>(std::errc::value_too_large),

        cannot_access_shared_library   = ELIBACC,

        operation_not_supported        = static_cast<code_int_t>(std::errc::operation_not_supported),

        operation_in_progress          = static_cast<code_int_t>(std::errc::operation_in_progress),
        stale_file_handle              = ESTALE,

        operation_canceled             = static_cast<code_int_t>(std::errc::operation_canceled),

        posix_end = std::numeric_limits<std::uint8_t>::max(),


        //Custom errors
        no_vulkan_devices,
        device_lacks_display_format,
        device_lacks_present_mode,
        device_lacks_suitable_mem_type,
        device_not_selected,
        device_not_initialized,
        device_lacks_necessary_queue_base,
        device_lacks_necessary_queue_last = device_lacks_necessary_queue_base + vk::queue_family::num_families - 1,
        
        window_not_found,
        window_already_exists,
        element_not_found,
        buffer_needs_changes_applied,
        invalid_image_initialization,
        descriptors_not_initialized,
        font_not_found,

        d2d_custom_end = 0xFFF,


        //GLFW errors
        window_system_not_initialized      = GLFW_NOT_INITIALIZED,       //__D2D_GLFW_TO_ERRC(GLFW_NOT_INITIALIZED),
        invalid_window_enum_argument       = GLFW_INVALID_ENUM,          //__D2D_GLFW_TO_ERRC(GLFW_INVALID_ENUM),
        invalid_window_argument            = GLFW_INVALID_VALUE,         //__D2D_GLFW_TO_ERRC(GLFW_INVALID_VALUE),
        out_of_memory_for_window           = GLFW_OUT_OF_MEMORY,         //__D2D_GLFW_TO_ERRC(GLFW_OUT_OF_MEMORY),
        vulkan_not_supported               = GLFW_API_UNAVAILABLE,       //__D2D_GLFW_TO_ERRC(GLFW_API_UNAVAILABLE),
        os_window_error                    = GLFW_PLATFORM_ERROR,        //__D2D_GLFW_TO_ERRC(GLFW_PLATFORM_ERROR),
        missing_pixel_format               = GLFW_FORMAT_UNAVAILABLE,    //__D2D_GLFW_TO_ERRC(GLFW_FORMAT_UNAVAILABLE),
        cannot_convert_clipboard           = GLFW_FORMAT_UNAVAILABLE,    //__D2D_GLFW_TO_ERRC(GLFW_FORMAT_UNAVAILABLE),
        missing_cursor_shape               = GLFW_CURSOR_UNAVAILABLE,    //__D2D_GLFW_TO_ERRC(GLFW_CURSOR_UNAVAILABLE),
        missing_window_feature             = GLFW_FEATURE_UNAVAILABLE,   //__D2D_GLFW_TO_ERRC(GLFW_FEATURE_UNAVAILABLE),
        window_feature_not_yet_implemented = GLFW_FEATURE_UNIMPLEMENTED, //__D2D_GLFW_TO_ERRC(GLFW_FEATURE_UNIMPLEMENTED)
        missing_window_platform            = GLFW_PLATFORM_UNAVAILABLE,  //__D2D_GLFW_TO_ERRC(GLFW_PLATFORM_UNAVAILABLE),

        //KTX errors
        invalid_texture_file_format              = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_DATA_ERROR),
        texture_file_is_a_pipe                   = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_ISPIPE),
        failed_to_open_texture_file              = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_OPEN_FAILED),
        texture_file_too_large                   = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_OVERFLOW),
        failed_to_read_texture_file              = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_READ_ERROR),
        failed_to_seek_texture_file              = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_SEEK_ERROR),
        unexpected_eof_in_texture_file           = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_UNEXPECTED_EOF),
        failed_to_write_texture_file             = __D2D_KTX_ERROR_TO_ERRC(KTX_FILE_WRITE_ERROR),
        invalid_gl_operation                     = __D2D_KTX_ERROR_TO_ERRC(KTX_GL_ERROR),
        invalid_texture_operation                = __D2D_KTX_ERROR_TO_ERRC(KTX_INVALID_OPERATION),
        invalid_texture_value                    = __D2D_KTX_ERROR_TO_ERRC(KTX_INVALID_VALUE),
        texture_function_not_found               = __D2D_KTX_ERROR_TO_ERRC(KTX_NOT_FOUND),
        out_of_memory_for_texture                = __D2D_KTX_ERROR_TO_ERRC(KTX_OUT_OF_MEMORY),
        texture_transcode_failed                 = __D2D_KTX_ERROR_TO_ERRC(KTX_TRANSCODE_FAILED),
        unknown_texture_file_format              = __D2D_KTX_ERROR_TO_ERRC(KTX_UNKNOWN_FILE_FORMAT),
        texture_type_not_supported               = __D2D_KTX_ERROR_TO_ERRC(KTX_UNSUPPORTED_TEXTURE_TYPE),
        texture_loading_feature_not_supported    = __D2D_KTX_ERROR_TO_ERRC(KTX_UNSUPPORTED_FEATURE),
        graphics_library_not_linked              = __D2D_KTX_ERROR_TO_ERRC(KTX_LIBRARY_NOT_LINKED),
        invalid_texture_size_after_decompression = __D2D_KTX_ERROR_TO_ERRC(KTX_DECOMPRESS_LENGTH_ERROR),
        invalid_texture_checksum                 = __D2D_KTX_ERROR_TO_ERRC(KTX_DECOMPRESS_CHECKSUM_ERROR),
        
        //Freetype errors
        // cannot_open_font                 = __D2D_FTERR_TO_ERRC(FT_Err_Cannot_Open_Resource),
        // unknown_font_file_format         = __D2D_FTERR_TO_ERRC(FT_Err_Unknown_File_Format),
        // invalid_font_file_format         = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_File_Format),
        // invalid_freetype_version         = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Version),
        // font_module_outdated             = __D2D_FTERR_TO_ERRC(FT_Err_Lower_Module_Version),
        // invalid_font_argument            = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Argument),
        // font_feature_not_yet_implemented = __D2D_FTERR_TO_ERRC(FT_Err_Unimplemented_Feature),
        // invalid_font_table               = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Table),
        // invalid_font_table_offset        = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Offset),
        // font_array_allocation_too_large  = __D2D_FTERR_TO_ERRC(FT_Err_Array_Too_Large),
        // missing_font_module              = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Module),
        // missing_font_property            = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Property),

        // invalid_glyph_index        = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Glyph_Index),
        // invalid_character_code     = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Character_Code),
        // invalid_glyph_format       = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Glyph_Format),
        // cannot_render_glyph_format = __D2D_FTERR_TO_ERRC(FT_Err_Cannot_Render_Glyph),
        // invalid_glyph_outline      = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Outline),
        // invalid_composite_glyph    = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Composite),
        // too_many_font_hints        = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Hints),
        // invalid_glyph_pixel_size   = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Pixel_Size),
        // invalid_svg_font           = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_SVG_Document),

        // invalid_font_object_handle   = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Handle),
        // invalid_font_library_handle  = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Library_Handle),
        // invalid_font_driver_handle   = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Driver_Handle),
        // invalid_font_face_handle     = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Face_Handle),
        // invalid_font_size_handle     = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Size_Handle),
        // invalid_glpyh_slot_handle    = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Slot_Handle),
        // invalid_font_char_map_handle = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_CharMap_Handle),
        // invalid_font_cache_handle    = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Cache_Handle),
        // invalid_font_stream_handle   = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Stream_Handle),

        // too_many_font_modules    = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Drivers),
        // too_many_font_extensions = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Extensions),

        // out_of_memory_for_font = __D2D_FTERR_TO_ERRC(FT_Err_Out_Of_Memory),
        // unlisted_font_object   = __D2D_FTERR_TO_ERRC(FT_Err_Unlisted_Object),

        // cannot_open_font_stream       = __D2D_FTERR_TO_ERRC(FT_Err_Cannot_Open_Stream),
        // invalid_font_stream_seek      = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Stream_Seek),
        // invalid_font_stream_skip      = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Stream_Skip),
        // invalid_font_stream_read      = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Stream_Read),
        // invalid_font_stream_operation = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Stream_Operation),
        // invalid_font_frame_operation  = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Frame_Operation),
        // invalid_font_frame_access     = __D2D_FTERR_TO_ERRC(FT_Err_Nested_Frame_Access),
        // invalid_font_frame_read       = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Frame_Read),

        // font_raster_uninitialized   = __D2D_FTERR_TO_ERRC(FT_Err_Raster_Uninitialized),
        // font_raster_corrupted       = __D2D_FTERR_TO_ERRC(FT_Err_Raster_Corrupted),
        // font_raster_overflow        = __D2D_FTERR_TO_ERRC(FT_Err_Raster_Overflow),
        // font_raster_negative_height = __D2D_FTERR_TO_ERRC(FT_Err_Raster_Negative_Height),

        // too_many_font_caches = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Caches),

        // invalid_truetype_opcode                   = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Opcode),
        // too_few_truetype_arguments                = __D2D_FTERR_TO_ERRC(FT_Err_Too_Few_Arguments),
        // truetype_stack_overflow                   = __D2D_FTERR_TO_ERRC(FT_Err_Stack_Overflow),
        // truetype_code_overflow                    = __D2D_FTERR_TO_ERRC(FT_Err_Code_Overflow),
        // invalid_truetype_argument                 = __D2D_FTERR_TO_ERRC(FT_Err_Bad_Argument),
        // truetype_division_by_zero                 = __D2D_FTERR_TO_ERRC(FT_Err_Divide_By_Zero),
        // invalid_truetype_reference                = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Reference),
        // truetype_debug_opcode                     = __D2D_FTERR_TO_ERRC(FT_Err_Debug_OpCode),
        // found_endf_opcode_in_truetype_stream      = __D2D_FTERR_TO_ERRC(FT_Err_ENDF_In_Exec_Stream),
        // nested_truetype_defs                      = __D2D_FTERR_TO_ERRC(FT_Err_Nested_DEFS),
        // invalid_truetype_code_range               = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_CodeRange),
        // truetype_excecution_too_long              = __D2D_FTERR_TO_ERRC(FT_Err_Execution_Too_Long),
        // too_many_truetype_function_defs           = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Function_Defs),
        // too_many_truetype_instruction_defs        = __D2D_FTERR_TO_ERRC(FT_Err_Too_Many_Instruction_Defs),
        // missing_sfnt_font_table                   = __D2D_FTERR_TO_ERRC(FT_Err_Table_Missing),
        // missing_truetype_horizontal_header_table  = __D2D_FTERR_TO_ERRC(FT_Err_Horiz_Header_Missing),
        // missing_truetype_locations_table          = __D2D_FTERR_TO_ERRC(FT_Err_Locations_Missing),
        // missing_truetype_name_table               = __D2D_FTERR_TO_ERRC(FT_Err_Name_Table_Missing),
        // missing_truetype_char_map_table           = __D2D_FTERR_TO_ERRC(FT_Err_CMap_Table_Missing),
        // missing_truetype_horizontal_metrics_table = __D2D_FTERR_TO_ERRC(FT_Err_Hmtx_Table_Missing),
        // missing_truetype_post_script_table        = __D2D_FTERR_TO_ERRC(FT_Err_Post_Table_Missing),
        // invalid_truetype_horizontal_metrics       = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Horiz_Metrics),
        // invalid_truetype_char_map_format          = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_CharMap_Format),
        // invalid_truetype_ppem_value               = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_PPem),
        // invalid_truetype_vertical_metrics         = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Vert_Metrics),
        // missing_truetype_context                  = __D2D_FTERR_TO_ERRC(FT_Err_Could_Not_Find_Context),
        // invalid_truetype_postscript_table_format  = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Post_Table_Format),
        // invalid_truetype_postscript_table         = __D2D_FTERR_TO_ERRC(FT_Err_Invalid_Post_Table),
        // def_in_glyf_bytecode                      = __D2D_FTERR_TO_ERRC(FT_Err_DEF_In_Glyf_Bytecode),
        // missing_truetype_bitmap                   = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Bitmap),
        // missing_truetype_svg_hooks                = __D2D_FTERR_TO_ERRC(FT_Err_Missing_SVG_Hooks),

        // cff_syntax_error              = __D2D_FTERR_TO_ERRC(FT_Err_Syntax_Error),
        // cff_argument_stack_underflow  = __D2D_FTERR_TO_ERRC(FT_Err_Stack_Underflow),
        // cff_ignore                    = __D2D_FTERR_TO_ERRC(FT_Err_Ignore),
        // cff_no_unicode_glyph_fount    = __D2D_FTERR_TO_ERRC(FT_Err_No_Unicode_Glyph_Name),
        // cff_glyph_too_big_for_hinting = __D2D_FTERR_TO_ERRC(FT_Err_Glyph_Too_Big),

        // missing_bdf_start_font_field   = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Startfont_Field),
        // missing_bdf_font_field         = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Font_Field),
        // missing_bdf_size_field         = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Size_Field),
        // missing_bdf_bounding_box_field = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Fontboundingbox_Field),
        // missing_bdf_chars_field        = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Chars_Field),
        // missing_bdf_start_char_field   = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Startchar_Field),
        // missing_bdf_encoding_field     = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Encoding_Field),
        // missing_bdf_bbx_field          = __D2D_FTERR_TO_ERRC(FT_Err_Missing_Bbx_Field),
        // bdf_bbx_too_big                = __D2D_FTERR_TO_ERRC(FT_Err_Bbx_Too_Big),
        // bdf_invalid_font_header        = __D2D_FTERR_TO_ERRC(FT_Err_Corrupted_Font_Header),
        // bdf_invalid_font_glyphs        = __D2D_FTERR_TO_ERRC(FT_Err_Corrupted_Font_Glyphs),



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
        num_unique_codes = 96,
        num_codes = num_unique_codes + num_duplicate_codes
    };
}

namespace d2d::error {
    constexpr frozen::unordered_map<code_int_t, std::string_view, code::num_unique_codes> code_descs = {
        //TODO
        {invalid_argument,          "Invalid argument passed to a non-graphical function"},
        {no_such_file_or_directory, "Requested file or directory does not exist"},


        {no_vulkan_devices,                                                                       "Could not find a graphics device with vulkan support"},
        {device_lacks_display_format,                                                             "The given device lacks the requested display format"},
        {device_lacks_present_mode,                                                               "The given device lacks a suitable present mode"},
        {device_lacks_suitable_mem_type,                                                          "The given device lacks the requested memory type"},
        {device_not_selected,                                                                     "A graphics device has not been selected"},
        {device_not_initialized,                                                                  "The selected device has not been initialized yet"},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(vk::queue_family::graphics), "The given device lacks a (required) graphics queue"},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(vk::queue_family::present),  "The given device lacks a (required) present queue"},
        
        {window_not_found,                   "The requested window was not found."},
        {window_already_exists,              "A window with the requested name already exists"},
        {element_not_found,                  "The requested element was not found."},
        {buffer_needs_changes_applied,       "Buffer is outdated and needs to have changes applied before being used again. Did you forget to call `apply_changes()`?"},
        {invalid_image_initialization,       "Invalid arguments passed when initializing the given image"},
        {descriptors_not_initialized,        "The given descriptors have not been initialized yet"},
        {font_not_found,                     "The requested font was not found. Did you forget to insert it into the window?"},

    
        {invalid_texture_file_format,              "The given KTX texture file contains invalid data"},
        {texture_file_is_a_pipe,                   "The given KTX texture file is actually a pipe, not a file"},
        {failed_to_open_texture_file,              "Failed to open the given KTX texture file"},
        {texture_file_too_large,                   "The given KTX texture file is too large"},
        {failed_to_read_texture_file,              "Failed to read the given KTX texture file"},
        {failed_to_seek_texture_file,              "Failed to seek the given KTX texture file"},
        {unexpected_eof_in_texture_file,           "Unexpected EOF encountered in the given KTX texture file"},
        {failed_to_write_texture_file,             "Failed to write to the given KTX texture file"},
        {invalid_gl_operation,                     "A GL operation failed"},
        {invalid_texture_operation,                "Invalid operation given the given KTX texture file in its current state"},
        {invalid_texture_value,                    "Invalid KTX texture parameter value encountered"},
        {texture_function_not_found,               "The requested KTX texture function was not found"},
        {out_of_memory_for_texture,                "A memory allocation for a KTX texture failed (most likely out of memory)"},
        {texture_transcode_failed,                 "Failed to transcode a block-compressed KTX texture"},
        {unknown_texture_file_format,              "Unknown KTX texture file format (i.e. it's not a KTX file)"},
        {texture_type_not_supported,               "The given KTX texture file specifies an unsupported texture type"},
        {texture_loading_feature_not_supported,    "The requested texture feature is not supported by KTX texture library"},
        {graphics_library_not_linked,              "Vulkan graphics library is not linked"},
        {invalid_texture_size_after_decompression, "Decompressed byte count for the given KTX texture does not match the expected byte count"},
        {invalid_texture_checksum,                 "Checksum for the given decompressed KTX texture does not match the expected checksum"},
        

        {window_system_not_initialized,      "Window system (GLFW) needs to be initialized first"},
        {invalid_window_enum_argument,       "Invalid enum argument passed to window system function"},
        {invalid_window_argument,            "Invalid argument passed to window system function"},
        {out_of_memory_for_window,           "A memory allocation for the window system failed (most likely out of memory)"},
        {vulkan_not_supported,               "Vulkan was not found on the system; It's either not supported or not installed"},
        {os_window_error,                    "A platform-specific error occured within the window system"},
        {missing_pixel_format,               "Either the requested display pixel format is not supported (if specified during window creation), or the clipboard contents could not be converted to the requested format (if specified during clipboard querying)"},
        {missing_cursor_shape,               "The requested standard cursor shape is not available"},
        {missing_window_feature,             "The requested window feature is not supported by the platform"},
        {window_feature_not_yet_implemented, "The requested window feature has not been implemented in the window system yet"},
        {missing_window_platform,            "The requested window platform was not found, or, if none was specifically requested, no supported platforms were found"},

        // {cannot_open_font,                 "Cannot open specified font file"},
        // {unknown_font_file_format,         "Unknown font file format"},
        // {invalid_font_file_format,         "Invalid font file format"},
        // {invalid_freetype_version,         "Invalid freetype version specified"},
        // {font_module_outdated,             "Outdated font module version specified"},
        // {invalid_font_argument,            "Invalid argument passed to font function"},
        // {font_feature_not_yet_implemented, "The requested font feature has not been implemented in Freetype yet"},
        // {invalid_font_table,               "Invalid font table"},
        // {invalid_font_table_offset,        "Invalid offset within font table"},
        // {font_array_allocation_too_large,  "A memory allocation for a font array failed because it is too large"},
        // {missing_font_module,              "Missing font module"},
        // {missing_font_property,            "Missing font property"},

        // {invalid_glyph_index,        "Invalid glyph index"},
        // {invalid_character_code,     "Invalid character code"},
        // {invalid_glyph_format,       "The glyph image format specified is not supported"},
        // {cannot_render_glyph_format, "Cannot render specified glyph format"},
        // {invalid_glyph_outline,      "Invalid glyph outline"},
        // {invalid_composite_glyph,    "Invalid composite glyph"},
        // {too_many_font_hints,        "Too many font hints specified"},
        // {invalid_glyph_pixel_size,   "Invalid glyph pixel size"},
        // {invalid_svg_font,           "Invalid font-related SVG document"},

        // {invalid_font_object_handle,   "Invalid font object handle"},
        // {invalid_font_library_handle,  "Invalid font library handle"},
        // {invalid_font_driver_handle,   "Invalid font driver handle"},
        // {invalid_font_face_handle,     "Invalid font face handle"},
        // {invalid_font_size_handle,     "Invalid font size handle"},
        // {invalid_glpyh_slot_handle,    "Invalid glpyh slot handle"},
        // {invalid_font_char_map_handle, "invalid font char map handle"},
        // {invalid_font_cache_handle,    "Invalid font cache handle"},
        // {invalid_font_stream_handle,   "Invalid font stream handle"},

        // {too_many_font_modules,    "Too many font modules"},
        // {too_many_font_extensions, "Too many font extensions"},

        // {out_of_memory_for_font, "A memory allocation for a font failed"},
        // {unlisted_font_object,   "Unlisted font object"},

        // {cannot_open_font_stream,       "Cannot open font stream"},
        // {invalid_font_stream_seek,      "Invalid font stream seek"},
        // {invalid_font_stream_skip,      "Invalid font stream skip"},
        // {invalid_font_stream_read,      "Invalid font stream read"},
        // {invalid_font_stream_operation, "Invalid font stream operation"},
        // {invalid_font_frame_operation,  "Invalid font frame operation"},
        // {invalid_font_frame_access,     "Invalid font frame access"},
        // {invalid_font_frame_read,       "Invalid font frame read"},

        // {font_raster_uninitialized,   "Font raster uninitialized"},
        // {font_raster_corrupted,       "Font raster corrupted"},
        // {font_raster_overflow,        "Font raster overflow"},
        // {font_raster_negative_height, "Font raster has negative height (while rastering)"},

        // {too_many_font_caches, "Too many registered font caches"},

        // {invalid_truetype_opcode,                   "Invalid truetype/SFNT opcode"},
        // {too_few_truetype_arguments,                "Too few arguements in given truetype/SFNT font"},
        // {truetype_stack_overflow,                   "stack overflow in truetype/SFNT font"},
        // {truetype_code_overflow,                    "code overflow in truetype/SFNT font"},
        // {invalid_truetype_argument,                 "Invalid argument in truetype/SFNT font"},
        // {truetype_division_by_zero,                 "Divison by 0 in truetype/SFNT font"},
        // {invalid_truetype_reference,                "Invalid reference in truetype/SFNT font"},
        // {truetype_debug_opcode,                     "Debug opcode found in truetype/SFNT font"},
        // {found_endf_opcode_in_truetype_stream,      "ENDF opcode found in truetype/SFNT execution stream"},
        // {nested_truetype_defs,                      "Nested DEFS in truetype/SFNT font"},
        // {invalid_truetype_code_range,               "Invalid code range in truetype/SFNT font"},
        // {truetype_excecution_too_long,              "Execution context in truetype/SFNT font too long"},
        // {too_many_truetype_function_defs,           "Too many function definitions in truetype/SFNT font"},
        // {too_many_truetype_instruction_defs,        "Too many instruction definitions in truetype/SFNT font"},
        // {missing_sfnt_font_table,                   "SFNT font table missing"},
        // {missing_truetype_horizontal_header_table,  "Missing horizontal header (hhea) in truetype/SFNT font"},
        // {missing_truetype_locations_table,          "Missing locations (loca) in truetype/SFNT font"},
        // {missing_truetype_name_table,               "Missing name table in truetype/SFNT font"},
        // {missing_truetype_char_map_table,           "Missing character map table (cmap) in truetype/SFNT font"},
        // {missing_truetype_horizontal_metrics_table, "Missing horizontal metrics (hmtx) in truetype/SFNT font"},
        // {missing_truetype_post_script_table,        "Missing PostScript (post) in truetype/SFNT font"},
        // {invalid_truetype_horizontal_metrics,       "Invalid horizontal metrics in truetype/SFNT font"},
        // {invalid_truetype_char_map_format,          "Invalid character map format (cmap) in truetype/SFNT font"},
        // {invalid_truetype_ppem_value,               "Invalid ppem value in truetype/SFNT font"},
        // {invalid_truetype_vertical_metrics,         "Invalid vertical metrics in truetype/SFNT font"},
        // {missing_truetype_context,                  "Missing truetype/SFNT font context"},
        // {invalid_truetype_postscript_table_format,  "Invalid PostScript table format (post) in truetype/SFNT font"},
        // {invalid_truetype_postscript_table,         "Invalid PostScript table (post) in truetype/SFNT font"},
        // {def_in_glyf_bytecode,                      "Found FDEF or IDEF opcode in truetype/SFNT font glyf bytecode"},
        // {missing_truetype_bitmap,                   "Missing bitmap in truetype/SFNT font strike"},
        // {missing_truetype_svg_hooks,                "Missing truetype/SFNT font SVG hooks"},

        // {cff_syntax_error,              "CFF font opcode syntax error"},
        // {cff_argument_stack_underflow,  "CFF font argument stack underflow"},
        // {cff_ignore,                    "CFF font ignore"},
        // {cff_no_unicode_glyph_fount,    "No unicode glyph name found in CFF font"},
        // {cff_glyph_too_big_for_hinting, "CFF font glyph is too big for hinting"},

        // {missing_bdf_start_font_field,   "Missing `STARTFONT` field in BDF font"},
        // {missing_bdf_font_field,         "Missing `FONT` field in BDF font"},
        // {missing_bdf_size_field,         "Missing `SIZE` field in BDF font"},
        // {missing_bdf_bounding_box_field, "Missing `FONTBOUNDINGBOX` field in BDF font"},
        // {missing_bdf_chars_field,        "Missing `CHARS` field in BDF font"},
        // {missing_bdf_start_char_field,   "Missing `STARTCHAR` field in BDF font"},
        // {missing_bdf_encoding_field,     "Missing `ENCODING` field in BDF font"},
        // {missing_bdf_bbx_field,          "Missing `BBX` field in BDF font"},
        // {bdf_bbx_too_big,                "`BBX` field in BDF font is too big"},
        // {bdf_invalid_font_header,        "Invalid BDF font header - it is either corrupted or missing fields"},
        // {bdf_invalid_font_glyphs,        "Invalid BDF font glyphs - it is either corrupted or missing fields"},


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


    constexpr frozen::unordered_map<code_int_t, int, code::num_unique_codes> code_errc_mappings = {
        {invalid_argument,          invalid_argument},
        {no_such_file_or_directory, no_such_file_or_directory},


        {no_vulkan_devices,                                                                       no_such_device},
        {device_lacks_display_format,                                                             no_such_device_or_address},
        {device_lacks_present_mode,                                                               no_such_device_or_address},
        {device_lacks_suitable_mem_type,                                                          no_such_device_or_address},
        {device_not_selected,                                                                     bad_file_descriptor},
        {device_not_initialized,                                                                  bad_file_descriptor},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(vk::queue_family::graphics), no_such_device_or_address},
        {device_lacks_necessary_queue_base + static_cast<code_int_t>(vk::queue_family::present),  no_such_device_or_address},
        
        {window_not_found,                   no_such_device_or_address},
        {window_already_exists,              invalid_argument},
        {element_not_found,                  no_such_device_or_address},
        {buffer_needs_changes_applied,       stale_file_handle},
        {invalid_image_initialization,       invalid_argument},
        {descriptors_not_initialized,        bad_file_descriptor},
        {font_not_found,                     no_such_device_or_address},


        {invalid_texture_file_format,              invalid_argument},
        {texture_file_is_a_pipe,                   broken_pipe},
        {failed_to_open_texture_file,              no_such_file_or_directory},
        {texture_file_too_large,                   file_too_large},
        {failed_to_read_texture_file,              io_error},
        {failed_to_seek_texture_file,              invalid_seek},
        {unexpected_eof_in_texture_file,           invalid_argument},
        {failed_to_write_texture_file,             io_error},
        {invalid_gl_operation,                     operation_canceled},
        {invalid_texture_operation,                operation_not_permitted},
        {invalid_texture_value,                    invalid_argument},
        {texture_function_not_found,               no_such_process},
        {out_of_memory_for_texture,                not_enough_memory},
        {texture_transcode_failed,                 operation_canceled},
        {unknown_texture_file_format,              invalid_argument},
        {texture_type_not_supported,               operation_not_supported},
        {texture_loading_feature_not_supported,    operation_not_supported},
        {graphics_library_not_linked,              cannot_access_shared_library},
        {invalid_texture_size_after_decompression, io_error},
        {invalid_texture_checksum,                 io_error},


        {window_system_not_initialized,      bad_file_descriptor},
        {invalid_window_enum_argument,       invalid_argument},
        {invalid_window_argument,            invalid_argument},
        {out_of_memory_for_window,           invalid_argument},
        {vulkan_not_supported,               operation_not_supported},
        {os_window_error,                    interrupted},
        {missing_pixel_format,               invalid_argument},
        {missing_cursor_shape,               invalid_argument},
        {missing_window_feature,             invalid_argument},
        {window_feature_not_yet_implemented, function_not_supported},
        {missing_window_platform,            operation_not_supported},


        {fence_or_query_not_complete, operation_in_progress},
        {vulkan_operation_timed_out,  operation_canceled},
        {vulkan_event_signaled,       operation_in_progress},
        {vulkan_event_unsignaled,     operation_canceled},
        {vulkan_array_too_small,      value_too_large},
        {swap_chain_out_of_date,      stale_file_handle},

        {vulkan_thread_idle,              operation_in_progress},
        {vulkan_thread_done,              operation_in_progress},
        {vulkan_operation_deferred,       operation_in_progress},
        {vulkan_operation_not_deferred,   operation_in_progress},
        {pipeline_should_be_compiled,     operation_in_progress},
        {missing_pipeline_cache_entry,    no_such_process},
        {shader_incompatible_with_device, no_such_device_or_address},
        
        {out_of_host_memory,                     not_enough_memory},
        {out_of_gpu_memory,                      not_enough_memory},
        {vulkan_object_initialization_failed,    operation_canceled},
        {vulkan_device_lost,                     device_or_resource_busy},
        {vulkan_memory_map_failed,               not_enough_memory},
        {missing_validation_layer,               operation_not_supported},
        {missing_vulkan_extension,               no_such_device_or_address},
        {missing_gpu_feature,                    no_such_device_or_address},
        {vulkan_version_not_suppoted,            operation_not_supported},
        {max_gpu_objects_reached,                too_many_files_open},
        {vulkan_format_not_supported,            operation_not_supported},
        {vulkan_pool_too_fragmented,             resource_unavailable_try_again},
        {vulkan_surface_lost,                    device_or_resource_busy},
        {window_in_use,                          device_or_resource_busy},
        {surface_out_of_date,                    stale_file_handle},
        {incompatible_display_and_image_layout,  operation_not_supported},
        {shader_failed_to_compile,               io_error},
        {vulkan_pool_allocation_failed,          not_enough_memory},
        {invalid_vulkan_handle,                  bad_file_descriptor},
        {descriptor_pool_too_fragmented,         not_enough_memory},
        {gpu_address_not_available,              bad_address},
        {no_exclusive_fullscreen_access,         operation_not_permitted},
        {invalid_vulkan_usage,                   invalid_argument},
        {no_resources_for_compression_available, device_or_resource_busy},
        {image_usage_not_supported,              operation_not_supported},
        {image_layout_not_supported,             operation_not_supported},
        {video_profile_operation_not_supported,  operation_not_supported},
        {video_profile_format_not_supported,     operation_not_supported},
        {video_codec_not_supported,              operation_not_supported},
        {video_std_header_not_supported,         operation_not_supported},
        {invalid_video_std_parameters,           invalid_argument},
        {gpu_operation_not_permitted,            operation_not_permitted},
        {not_enough_space_for_return_value,      value_too_large},
        {invalid_format_modifier,                invalid_argument},
        {unknown_vulkan_error,                   operation_canceled},
    };
}

OL_RESULT_DECLARE_AS_ERROR_CODE(d2d::error, code, &(ol::error_category_msg_map<decltype(code_descs), code_descs>), &(ol::error_category_errc_map<decltype(code_errc_mappings), code_errc_mappings>), duo2d)


namespace d2d {
    using errc = error::code;

    template<typename T>
    using result = ol::result<T, errc>;
}


#define __D2D_WEAK_PTR_TRY_LOCK(lhs, weak_ptr) auto lhs = weak_ptr.lock(); if(!lhs) return ::d2d::errc::device_not_initialized;


#define __D2D_VULKAN_VERIFY(fn) if(VkResult r = fn)       [[unlikely]] return static_cast<d2d::errc>(__D2D_VKRESULT_TO_ERRC(r));
#define __D2D_KTX_VERIFY(fn)    if(KTX_error_code r = fn) [[unlikely]] return static_cast<d2d::errc>(__D2D_KTX_ERROR_TO_ERRC(r));
#define __D2D_FT_VERIFY(fn)     if(FT_Error r = fn)       [[unlikely]] return static_cast<d2d::errc>(__D2D_FTERR_TO_ERRC(r));
#define __D2D_GLFW_VERIFY(cond) if(!cond)                 [[unlikely]] return static_cast<d2d::errc>(::d2d::error::impl::get_glfw_err());


namespace d2d::error::impl {
    constexpr std::size_t glfw_desc_size = 1024; //_GLFW_MESSAGE_SIZE

    inline std::array<char, impl::glfw_desc_size>& last_glfw_desc_array() noexcept {
        static std::array<char, impl::glfw_desc_size> last_glfw_char_arr{};
        return last_glfw_char_arr;
    }

    inline int get_glfw_err() noexcept {
        char const* last_glfw_char_ptr = nullptr;
        int ret = glfwGetError(&last_glfw_char_ptr);
        if(!last_glfw_char_ptr) return ret;
        std::memcpy(last_glfw_desc_array().data(), last_glfw_char_ptr, glfw_desc_size);
        return ret;
    }
}

namespace d2d::error {
    inline std::string_view last_glfw_desc() noexcept {
        char const* const desc_ptr = impl::last_glfw_desc_array().data();
        return std::string_view{desc_ptr, static_cast<char const*>(std::memchr(desc_ptr, '\0', impl::glfw_desc_size))};
    }
}