#pragma once 
#include <mutex>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include "sirius/input/combination.hpp"


namespace acma::input::impl {
    struct window_info {
        combination current_combo;
        void* window_ptr;
        std::mutex combo_mutex;
    };
}


namespace acma::input::impl {
    inline std::unordered_map<GLFWwindow*, window_info>& glfw_window_map() {
        static std::unordered_map<GLFWwindow*, window_info> m{};
        return m;
    }
}