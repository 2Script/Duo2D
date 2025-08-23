#pragma once 
#include <mutex>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include "Duo2D/input/combination.hpp"


namespace d2d::input::impl {
    struct window_info {
        combination current_combo;
        void* window_ptr;
        std::mutex combo_mutex;
    };
}


namespace d2d::input::impl {
    inline std::unordered_map<GLFWwindow*, window_info>& glfw_window_map() {
        static std::unordered_map<GLFWwindow*, window_info> m{};
        return m;
    }
}