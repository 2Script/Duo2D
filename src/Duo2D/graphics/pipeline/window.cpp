#include "Duo2D/graphics/pipeline/window.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/shader_module.hpp"
#include "Duo2D/graphics/pipeline/surface.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/shaders/triangle.hpp"

namespace d2d {
    result<window> window::create(std::string_view title, std::size_t width, std::size_t height, instance const& i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        window ret{glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
        __D2D_GLFW_VERIFY(ret.handle);

        //Create surface
        __D2D_TRY_MAKE(ret.window_surface, make<surface>(ret, i), s);

        return ret;
    }
}

namespace d2d {
    result<void> window::initialize_swap(logical_device& logi_deivce, physical_device& phys_device) noexcept {
        if(window_swap_chain) 
            return result<void>{std::in_place_type<void>}; //return error::swap_chain_already_initialized;
        
        //Create swap chain
        __D2D_TRY_MAKE(window_swap_chain, make<swap_chain>(logi_deivce, phys_device, window_surface, *this), s);

        //Create shaders (TEMP: hardcoded make arguments)
        __D2D_TRY_MAKE(shader_module triangle_vert, make<shader_module>(logi_deivce, "main", shaders::triangle::vert, 1504, VK_SHADER_STAGE_VERTEX_BIT), tv);
        __D2D_TRY_MAKE(shader_module triangle_frag, make<shader_module>(logi_deivce, "main", shaders::triangle::frag, 572, VK_SHADER_STAGE_FRAGMENT_BIT), tf);
        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {triangle_vert.stage_info(), triangle_frag.stage_info()};

        //Create pipeline
        __D2D_TRY_MAKE(window_pipeline, make<pipeline>(logi_deivce, shader_stages), p)


        return result<void>{std::in_place_type<void>};
    }
}