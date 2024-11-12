#include "Duo2D/graphics/pipeline/window.hpp"

#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/image_view.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/shader_module.hpp"
#include "Duo2D/graphics/pipeline/surface.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/sync/semaphore.hpp"
#include "Duo2D/hardware/device/queue_family.hpp"
#include "Duo2D/shaders/vertex2.hpp"

namespace d2d {
    result<window> window::create(std::string_view title, std::size_t width, std::size_t height, instance const& i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        window ret{glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
        __D2D_GLFW_VERIFY(ret.handle);

        //Create surface
        __D2D_TRY_MAKE(ret._surface, make<surface>(ret, i), s);

        return ret;
    }
}

namespace d2d {
    result<void> window::initialize(logical_device& logi_device, physical_device& phys_device) noexcept {
        if(_swap_chain) 
            return result<void>{std::in_place_type<void>}; //return error::window_already_initialized;

        logi_device_ptr = std::addressof(logi_device);
        phys_device_ptr = std::addressof(phys_device);
        
        //Create render pass
        __D2D_TRY_MAKE(_render_pass, make<render_pass>(logi_device), rp);

        //Create swap chain
        __D2D_TRY_MAKE(_swap_chain, make<swap_chain>(logi_device, phys_device, _render_pass, _surface, *this), s);

        //Create shaders (TEMP: hardcoded make arguments)
        __D2D_TRY_MAKE(shader_module triangle_vert, make<shader_module>(logi_device, shaders::vertex2::vert, VK_SHADER_STAGE_VERTEX_BIT), tv);
        __D2D_TRY_MAKE(shader_module triangle_frag, make<shader_module>(logi_device, shaders::vertex2::frag, VK_SHADER_STAGE_FRAGMENT_BIT), tf);
        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {triangle_vert.stage_info(), triangle_frag.stage_info()};

        //Create pipeline
        __D2D_TRY_MAKE(_pipeline, make<pipeline>(logi_device, _render_pass, shader_stages), p);

        //Create command pool
        __D2D_TRY_MAKE(_command_pool, make<command_pool>(logi_device, phys_device), cp);

        for(std::size_t i = 0; i < frames_in_flight; ++i) {
            //Create command buffers
            __D2D_TRY_MAKE(command_buffers[i], make<command_buffer>(logi_device, _command_pool), cb);

            //Create fences & sempahores
            __D2D_TRY_MAKE(render_fences[i], make<fence>(logi_device), f);
            __D2D_TRY_MAKE(image_available_semaphores[i], make<semaphore>(logi_device), ia);
            __D2D_TRY_MAKE(cmd_buffer_finished_semaphores[i], make<semaphore>(logi_device), cbf);
        }

        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    result<void> window::render() noexcept {
        render_fences[frame_idx].wait();
        render_fences[frame_idx].reset();

        uint32_t image_index;
        VkResult nir = vkAcquireNextImageKHR(*logi_device_ptr, _swap_chain, UINT64_MAX, image_available_semaphores[frame_idx], VK_NULL_HANDLE, &image_index);
        
        //Re-create swap chain if needed 
        switch(nir) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR: {
            __D2D_TRY_MAKE(_swap_chain, make<swap_chain>(*logi_device_ptr, *phys_device_ptr, _render_pass, _surface, *this), s);
            return result<void>{std::in_place_type<void>};
        }
        default: 
            return static_cast<errc>(nir);
        }

        command_buffers[frame_idx].reset();
        command_buffers[frame_idx].record(*this, image_index);

        constexpr static std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pWaitDstStageMask = wait_stages.data();
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_semaphores[frame_idx];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &cmd_buffer_finished_semaphores[frame_idx];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers[frame_idx];
        __D2D_VULKAN_VERIFY(vkQueueSubmit(logi_device_ptr->queues[queue_family::graphics], 1, &submit_info, render_fences[frame_idx]));

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &cmd_buffer_finished_semaphores[frame_idx];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &_swap_chain;
        present_info.pImageIndices = &image_index;
        __D2D_VULKAN_VERIFY(vkQueuePresentKHR(logi_device_ptr->queues[queue_family::present], &present_info));

        frame_idx = (frame_idx + 1) % frames_in_flight;
        return result<void>{std::in_place_type<void>};
    }
}
