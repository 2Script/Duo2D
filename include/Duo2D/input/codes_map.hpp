#pragma once
#include <array>
#include <cstdint>

#include <GLFW/glfw3.h>

#include "Duo2D/input/code.hpp"


namespace d2d::input {
    //Maps GLFW input codes to HID
    constexpr static std::array<code_t, GLFW_KEY_LAST + 1> codes_map = []() noexcept {
        std::array<code_t, GLFW_KEY_LAST + 1> mapping{};
        
        mapping[GLFW_MOUSE_BUTTON_1] = mouse_code::button_1;
        mapping[GLFW_MOUSE_BUTTON_2] = mouse_code::button_2;
        mapping[GLFW_MOUSE_BUTTON_3] = mouse_code::button_3;
        mapping[GLFW_MOUSE_BUTTON_4] = mouse_code::button_4;
        mapping[GLFW_MOUSE_BUTTON_5] = mouse_code::button_5;
        mapping[GLFW_MOUSE_BUTTON_6] = mouse_code::button_6;
        mapping[GLFW_MOUSE_BUTTON_7] = mouse_code::button_7;
        mapping[GLFW_MOUSE_BUTTON_8] = mouse_code::button_8;


        mapping[GLFW_KEY_SPACE        ] = key_code::kb_spacebar;
        mapping[GLFW_KEY_APOSTROPHE   ] = key_code::kb_apostrophe;
        mapping[GLFW_KEY_COMMA        ] = key_code::kb_comma;
        mapping[GLFW_KEY_MINUS        ] = key_code::kb_minus;
        mapping[GLFW_KEY_PERIOD       ] = key_code::kb_period;
        mapping[GLFW_KEY_SLASH        ] = key_code::kb_forward_slash;
        mapping[GLFW_KEY_0            ] = key_code::kb_0;
        mapping[GLFW_KEY_1            ] = key_code::kb_1;
        mapping[GLFW_KEY_2            ] = key_code::kb_2;
        mapping[GLFW_KEY_3            ] = key_code::kb_3;
        mapping[GLFW_KEY_4            ] = key_code::kb_4;
        mapping[GLFW_KEY_5            ] = key_code::kb_3;
        mapping[GLFW_KEY_6            ] = key_code::kb_3;
        mapping[GLFW_KEY_7            ] = key_code::kb_3;
        mapping[GLFW_KEY_8            ] = key_code::kb_3;
        mapping[GLFW_KEY_9            ] = key_code::kb_3;
        mapping[GLFW_KEY_SEMICOLON    ] = key_code::kb_semicolon;
        mapping[GLFW_KEY_EQUAL        ] = key_code::kb_equals;
        mapping[GLFW_KEY_A            ] = key_code::kb_a;
        mapping[GLFW_KEY_B            ] = key_code::kb_b;
        mapping[GLFW_KEY_C            ] = key_code::kb_c;
        mapping[GLFW_KEY_D            ] = key_code::kb_d;
        mapping[GLFW_KEY_E            ] = key_code::kb_e;
        mapping[GLFW_KEY_F            ] = key_code::kb_f;
        mapping[GLFW_KEY_G            ] = key_code::kb_g;
        mapping[GLFW_KEY_H            ] = key_code::kb_h;
        mapping[GLFW_KEY_I            ] = key_code::kb_i;
        mapping[GLFW_KEY_J            ] = key_code::kb_j;
        mapping[GLFW_KEY_K            ] = key_code::kb_k;
        mapping[GLFW_KEY_L            ] = key_code::kb_l;
        mapping[GLFW_KEY_M            ] = key_code::kb_m;
        mapping[GLFW_KEY_N            ] = key_code::kb_n;
        mapping[GLFW_KEY_O            ] = key_code::kb_o;
        mapping[GLFW_KEY_P            ] = key_code::kb_p;
        mapping[GLFW_KEY_Q            ] = key_code::kb_q;
        mapping[GLFW_KEY_R            ] = key_code::kb_r;
        mapping[GLFW_KEY_S            ] = key_code::kb_s;
        mapping[GLFW_KEY_T            ] = key_code::kb_t;
        mapping[GLFW_KEY_U            ] = key_code::kb_u;
        mapping[GLFW_KEY_V            ] = key_code::kb_u;
        mapping[GLFW_KEY_W            ] = key_code::kb_w;
        mapping[GLFW_KEY_X            ] = key_code::kb_x;
        mapping[GLFW_KEY_Y            ] = key_code::kb_y;
        mapping[GLFW_KEY_Z            ] = key_code::kb_z;
        mapping[GLFW_KEY_LEFT_BRACKET ] = key_code::kb_left_bracket;
        mapping[GLFW_KEY_BACKSLASH    ] = key_code::kb_backslash;
        mapping[GLFW_KEY_RIGHT_BRACKET] = key_code::kb_right_bracket;
        mapping[GLFW_KEY_GRAVE_ACCENT ] = key_code::kb_grave;
        mapping[GLFW_KEY_WORLD_1      ] = key_code::kb_international_number_sign; //?
        mapping[GLFW_KEY_WORLD_2      ] = key_code::kb_international_backslash;   //?
        mapping[GLFW_KEY_ESCAPE       ] = key_code::kb_escape;
        mapping[GLFW_KEY_ENTER        ] = key_code::kb_enter;
        mapping[GLFW_KEY_TAB          ] = key_code::kb_tab;
        mapping[GLFW_KEY_BACKSPACE    ] = key_code::kb_backspace;
        mapping[GLFW_KEY_INSERT       ] = key_code::kb_insert;
        mapping[GLFW_KEY_DELETE       ] = key_code::kb_delete;
        mapping[GLFW_KEY_RIGHT        ] = key_code::kb_right_arrow;
        mapping[GLFW_KEY_LEFT         ] = key_code::kb_left_arrow;
        mapping[GLFW_KEY_DOWN         ] = key_code::kb_down_arrow;
        mapping[GLFW_KEY_UP           ] = key_code::kb_up_arrow;
        mapping[GLFW_KEY_PAGE_UP      ] = key_code::kb_page_up;
        mapping[GLFW_KEY_PAGE_DOWN    ] = key_code::kb_page_down;
        mapping[GLFW_KEY_HOME         ] = key_code::kb_home;
        mapping[GLFW_KEY_END          ] = key_code::kb_end;
        mapping[GLFW_KEY_CAPS_LOCK    ] = key_code::kb_caps_lock;
        mapping[GLFW_KEY_SCROLL_LOCK  ] = key_code::kb_scroll_lock;
        mapping[GLFW_KEY_NUM_LOCK     ] = key_code::kp_num_lock;
        mapping[GLFW_KEY_PRINT_SCREEN ] = key_code::kb_print_screen;
        mapping[GLFW_KEY_PAUSE        ] = key_code::kb_pause;
        mapping[GLFW_KEY_F1           ] = key_code::kb_f1;
        mapping[GLFW_KEY_F2           ] = key_code::kb_f2;
        mapping[GLFW_KEY_F3           ] = key_code::kb_f3;
        mapping[GLFW_KEY_F4           ] = key_code::kb_f4;
        mapping[GLFW_KEY_F5           ] = key_code::kb_f5;
        mapping[GLFW_KEY_F6           ] = key_code::kb_f6;
        mapping[GLFW_KEY_F7           ] = key_code::kb_f7;
        mapping[GLFW_KEY_F8           ] = key_code::kb_f8;
        mapping[GLFW_KEY_F9           ] = key_code::kb_f9;
        mapping[GLFW_KEY_F10          ] = key_code::kb_f10;
        mapping[GLFW_KEY_F11          ] = key_code::kb_f11;
        mapping[GLFW_KEY_F12          ] = key_code::kb_f12;
        mapping[GLFW_KEY_F13          ] = key_code::kb_f13;
        mapping[GLFW_KEY_F14          ] = key_code::kb_f14;
        mapping[GLFW_KEY_F15          ] = key_code::kb_f15;
        mapping[GLFW_KEY_F16          ] = key_code::kb_f16;
        mapping[GLFW_KEY_F17          ] = key_code::kb_f17;
        mapping[GLFW_KEY_F18          ] = key_code::kb_f18;
        mapping[GLFW_KEY_F19          ] = key_code::kb_f19;
        mapping[GLFW_KEY_F20          ] = key_code::kb_f20;
        mapping[GLFW_KEY_F21          ] = key_code::kb_f21;
        mapping[GLFW_KEY_F22          ] = key_code::kb_f22;
        mapping[GLFW_KEY_F23          ] = key_code::kb_f23;
        mapping[GLFW_KEY_F24          ] = key_code::kb_f24;
        mapping[GLFW_KEY_F25          ] = key_code::reserved_2_2; //Does this actually exist or is this just a troll?
        mapping[GLFW_KEY_KP_0         ] = key_code::kp_0;
        mapping[GLFW_KEY_KP_1         ] = key_code::kp_1;
        mapping[GLFW_KEY_KP_2         ] = key_code::kp_2;
        mapping[GLFW_KEY_KP_3         ] = key_code::kp_3;
        mapping[GLFW_KEY_KP_4         ] = key_code::kp_4;
        mapping[GLFW_KEY_KP_5         ] = key_code::kp_5;
        mapping[GLFW_KEY_KP_6         ] = key_code::kp_6;
        mapping[GLFW_KEY_KP_7         ] = key_code::kp_7;
        mapping[GLFW_KEY_KP_8         ] = key_code::kp_8;
        mapping[GLFW_KEY_KP_9         ] = key_code::kp_9;
        mapping[GLFW_KEY_KP_DECIMAL   ] = key_code::kp_decimal;
        mapping[GLFW_KEY_KP_DIVIDE    ] = key_code::kp_divide;
        mapping[GLFW_KEY_KP_MULTIPLY  ] = key_code::kp_multiply;
        mapping[GLFW_KEY_KP_SUBTRACT  ] = key_code::kp_subtract;
        mapping[GLFW_KEY_KP_ADD       ] = key_code::kp_add;
        mapping[GLFW_KEY_KP_ENTER     ] = key_code::kp_enter;
        mapping[GLFW_KEY_KP_EQUAL     ] = key_code::kp_equals;
        mapping[GLFW_KEY_LEFT_SHIFT   ] = key_code::kb_left_shift;
        mapping[GLFW_KEY_LEFT_CONTROL ] = key_code::kb_left_ctrl;
        mapping[GLFW_KEY_LEFT_ALT     ] = key_code::kb_left_alt;
        mapping[GLFW_KEY_LEFT_SUPER   ] = key_code::kb_left_super;
        mapping[GLFW_KEY_RIGHT_SHIFT  ] = key_code::kb_right_shift;
        mapping[GLFW_KEY_RIGHT_CONTROL] = key_code::kb_right_ctrl;
        mapping[GLFW_KEY_RIGHT_ALT    ] = key_code::kb_right_alt;
        mapping[GLFW_KEY_RIGHT_SUPER  ] = key_code::kb_right_super;
        mapping[GLFW_KEY_MENU         ] = key_code::kb_menu;

        return mapping;
    }();
}