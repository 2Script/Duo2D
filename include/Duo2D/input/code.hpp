#pragma once
#include <climits>
#include <cstdint>
#include <limits>


namespace d2d::input {
    using code_t = std::uint8_t;

    constexpr static std::size_t num_codes = (std::numeric_limits<code_t>::max() + 1) - (sizeof(code_t) * CHAR_BIT);
}

namespace d2d::input {
    namespace key_code {
    enum : code_t {
        none,

        
        kb_error_roll_over,
        kb_error_post,
        kb_error_undefined,

        
        kb_a,
        kb_b,
        kb_c,
        kb_d,
        kb_e,
        kb_f,
        kb_g,
        kb_h,
        kb_i,
        kb_j,
        kb_k,
        kb_l,
        kb_m,
        kb_n,
        kb_o,
        kb_p,
        kb_q,
        kb_r,
        kb_s,
        kb_t,
        kb_u,
        kb_v,
        kb_w,
        kb_x,
        kb_y,
        kb_z,

        kb_1,
        kb_2,
        kb_3,
        kb_4,
        kb_5,
        kb_6,
        kb_7,
        kb_8,
        kb_9,
        kb_0,

        kb_enter,
        kb_escape,
        kb_backspace,
        kb_tab,
        kb_spacebar,
        kb_minus,
        kb_equals,
        kb_left_bracket,
        kb_right_bracket,
        kb_backslash,
        kb_international_number_sign,
        kb_semicolon,
        kb_apostrophe,
        kb_grave,
        kb_comma,
        kb_period,
        kb_forward_slash,
        kb_caps_lock,
        
        kb_f1,
        kb_f2,
        kb_f3,
        kb_f4,
        kb_f5,
        kb_f6,
        kb_f7,
        kb_f8,
        kb_f9,
        kb_f10,
        kb_f11,
        kb_f12,


        kb_print_screen,
        kb_scroll_lock,
        kb_pause,
        kb_insert,
        kb_home,
        kb_page_up,
        kb_delete,
        kb_end,
        kb_page_down,
        kb_right_arrow,
        kb_left_arrow,
        kb_down_arrow,
        kb_up_arrow,


        kp_num_lock,
        kp_divide,
        kp_multiply,
        kp_subtract,
        kp_add,
        kp_enter,
        kp_1,
        kp_2,
        kp_3,
        kp_4,
        kp_5,
        kp_6,
        kp_7,
        kp_8,
        kp_9,
        kp_0,
        kp_decimal,


        kb_international_backslash,
        kb_application,
        kb_power,


        kp_equals,
       
        
        kb_f13,
        kb_f14,
        kb_f15,
        kb_f16,
        kb_f17,
        kb_f18,
        kb_f19,
        kb_f20,
        kb_f21,
        kb_f22,
        kb_f23,
        kb_f24,
        
        kb_execute,
        kb_help,
        kb_menu,
        kb_select,
        kb_stop,
        kb_again,
        kb_undo,
        kb_cut,
        kb_copy,
        kb_paste,
        kb_find,
       
        kb_mute,
        kb_volume_up,
        kb_volume_down,
       
        kb_locking_caps_lock,
        kb_locking_num_lock,
        kb_locking_scroll_lock,
        

        kp_international_period,
        kp_alternate_equals,
        kb_international_unique_1,
        kb_international_unique_2,
        kb_international_unique_3,
        kb_international_unique_4,
        kb_international_unique_5,
        kb_international_unique_6,
        kb_international_unique_7,
        kb_international_unique_8,
        kb_international_unique_9,
        
        kb_lang_1,
        kb_lang_2,
        kb_lang_3,
        kb_lang_4,
        kb_lang_5,
        kb_lang_6,
        kb_lang_7,
        kb_lang_8,
        kb_lang_9,
        

        kb_alternate_erase,
        kb_sys_req,
        kb_cancel,
        kb_clear,
        kb_prior,
        kb_alternate_return,
        kb_separator,
        kb_out,
        kb_oper,
        kb_alternate_clear,
        kb_cr_sel,
        kb_ex_sel,
        
        
        reserved_1_1,
        reserved_1_2,
        reserved_1_3,
        reserved_1_4,
        reserved_1_5,
        reserved_1_6,
        reserved_1_7,
        reserved_1_8,
        reserved_1_9,
        reserved_1_10,
        reserved_1_11,
        
        
        kp_00,
        kp_000,
        universal_thousands_separator,
        universal_decimal_separator,
        universal_currency_unit,
        universal_currency_sub_unit,
        
        kp_left_parenthesis,
        kp_right_parenthesis,
        kp_left_bracket,
        kp_right_bracket,
        kp_tab,
        kp_backspace,
        
        kp_a,
        kp_b,
        kp_c,
        kp_d,
        kp_e,
        kp_f,
        
        kp_xor,
        kp_power,
        kp_mod,
        kp_left_angle_bracket,
        kp_right_angle_bracket,
        kp_bitwise_and,
        kp_logical_and,
        kp_bitwise_or,
        kp_logical_or,
        kp_colon,
        kp_number_sign,
        kp_space,
        kp_at,
        
        kp_exclamation_mark,
        kp_memory_store,
        kp_memory_recall,
        kp_memory_clear,
        kp_memory_add,
        kp_memory_subtract,
        kp_memory_multiply,
        kp_memory_divide,
        kp_negate,
        kp_clear,
        kp_clear_entry,
        kp_binary_mode,
        kp_octal_mode,
        kp_decimal_mode,
        kp_hexadecimal_mode,
        
        
        reserved_2_1,
        reserved_2_2,
        
        
        kb_left_ctrl,
        kb_left_shift,
        kb_left_alt,
        kb_left_super,
        
        kb_right_ctrl,
        kb_right_shift,
        kb_right_alt,
        kb_right_super,
        
        
        reserved_3_1,
        reserved_3_2,
        reserved_3_3,
        reserved_3_4,
        reserved_3_5,
        reserved_3_6,
        reserved_3_7,
        reserved_3_8,
        reserved_3_9,
        reserved_3_10,
        reserved_3_11,
        reserved_3_12,
        reserved_3_13,
        reserved_3_14,
        reserved_3_15,
        reserved_3_16,

        //Reserved for main input in combination
        //reserved_4_1,
        //reserved_4_2,
        //reserved_4_3,
        //reserved_4_4,
        //reserved_4_5,
        //reserved_4_6,
        //reserved_4_7,
        //reserved_4_8,

    };
    }
}


namespace d2d::input {
    namespace mouse_code {
    enum : code_t {
        button_1 = key_code::reserved_1_1,
        button_2 = key_code::reserved_1_2,
        button_3 = key_code::reserved_1_3,
        button_4 = key_code::reserved_1_4,
        button_5 = key_code::reserved_1_5,
        button_6 = key_code::reserved_1_6,
        button_7 = key_code::reserved_1_7,
        button_8 = key_code::reserved_1_8,

        scroll_right = key_code::reserved_3_1,
        scroll_left  = key_code::reserved_3_2,
        scroll_down  = key_code::reserved_3_3,
        scroll_up    = key_code::reserved_3_4,

        move = key_code::reserved_2_1,
    };
    }
}

namespace d2d::input {
    namespace generic_code {
    enum : code_t { 
        any = key_code::reserved_2_2,
    };
    }
}