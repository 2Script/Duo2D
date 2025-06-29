#include "Duo2D/core/window.hpp"

#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <memory>
#include <result/verify.h>
#include <type_traits>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/prim/glyph.hpp"
#include "Duo2D/graphics/prim/debug_rect.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/vulkan/device/queue_family.hpp"

namespace d2d {
    result<window> window::create(std::string_view title, std::size_t width, std::size_t height, std::shared_ptr<vk::instance> i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        window ret{glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
        __D2D_GLFW_VERIFY(ret.handle);

        //Create surface
        __D2D_TRY_MAKE(ret._surface, make<vk::surface>(ret, i), s);

        return ret;
    }
}

namespace d2d {
    result<void> window::initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device) noexcept {
        if(_swap_chain) 
            return {}; //return error::window_already_initialized;

        logi_device_ptr = logi_device;
        phys_device_ptr = phys_device;

        //Create command pool
        vk::command_pool c;
        RESULT_TRY_MOVE(c, make<vk::command_pool>(logi_device, phys_device));
        copy_cmd_pool_ptr = std::make_shared<vk::command_pool>(std::move(c));
        
        //Create render pass
        __D2D_TRY_MAKE(_render_pass, make<vk::render_pass>(logi_device), rp);

        //Create swap chain
        __D2D_TRY_MAKE(_swap_chain, make<vk::swap_chain>(logi_device, phys_device, _render_pass, _surface, *this), s);

        for(std::size_t i = 0; i < impl::frames_in_flight; ++i) {
            //Create command buffers
            __D2D_TRY_MAKE(command_buffers[i], make<vk::command_buffer>(logi_device, copy_cmd_pool_ptr), cb);

            //Create fences & sempahores
            __D2D_TRY_MAKE(render_fences[i], make<vk::fence>(logi_device), f);
            __D2D_TRY_MAKE(frame_semaphores[semaphore_type::image_available][i], make<vk::semaphore>(logi_device), ia);
        }
        
        //Create submit sempahores
        submit_semaphores.reserve(_swap_chain.image_count);
        for(std::size_t i = 0; i < _swap_chain.image_count; ++i) {
            RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(logi_device), submit_semaphore);
            submit_semaphores.push_back(*std::move(submit_semaphore));
        }

        __D2D_TRY_MAKE((*static_cast<vk::renderable_tuple<impl::frames_in_flight, styled_rect, debug_rect, clone_rect, glyph>*>(this)), (make<vk::renderable_tuple<impl::frames_in_flight, styled_rect, debug_rect, clone_rect, glyph>>(logi_device, phys_device, _render_pass)), rb);

        //update uniform buffer
        std::memcpy(&uniform_map<clone_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
        std::memcpy(&uniform_map<styled_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
        std::memcpy(&uniform_map<debug_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
        std::memcpy(&uniform_map<glyph>()[frame_idx], &_swap_chain.extent, sizeof(extent2));

        return {};
    }
}

namespace d2d {
    result<void> window::render() noexcept {
        //wait for rendering to finish last frame
        RESULT_VERIFY(render_fences[frame_idx].wait());

        uint32_t image_index;
        VkResult nir = vkAcquireNextImageKHR(*logi_device_ptr, _swap_chain, UINT64_MAX, frame_semaphores[semaphore_type::image_available][frame_idx], VK_NULL_HANDLE, &image_index);
        
        //Re-create swap chain if needed 
        switch(nir) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR: {
            __D2D_TRY_MAKE(_swap_chain, make<vk::swap_chain>(logi_device_ptr, phys_device_ptr, _render_pass, _surface, *this), s);
            std::memcpy(&uniform_map<clone_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
            std::memcpy(&uniform_map<styled_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
            std::memcpy(&uniform_map<debug_rect>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
            std::memcpy(&uniform_map<glyph>()[frame_idx], &_swap_chain.extent, sizeof(extent2));
            return {};
        }
        default: 
            return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(nir));
        }

        if(empty<styled_rect>() && empty<debug_rect>())
            return {};

        


        RESULT_VERIFY(render_fences[frame_idx].reset());

        RESULT_VERIFY(command_buffers[frame_idx].reset());
        RESULT_VERIFY(command_buffers[frame_idx].render_begin(_swap_chain, _render_pass, image_index));
        RESULT_VERIFY(command_buffers[frame_idx].draw<clone_rect>(*this));
        RESULT_VERIFY(command_buffers[frame_idx].draw<styled_rect>(*this));
        RESULT_VERIFY(command_buffers[frame_idx].draw<debug_rect>(*this));
        RESULT_VERIFY(command_buffers[frame_idx].draw<glyph>(*this));
        RESULT_VERIFY(command_buffers[frame_idx].render_end());

        constexpr static std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame_semaphores[semaphore_type::image_available][frame_idx],
            .pWaitDstStageMask = wait_stages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffers[frame_idx],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &submit_semaphores[image_index],
        };
        __D2D_VULKAN_VERIFY(vkQueueSubmit(logi_device_ptr->queues[vk::queue_family::graphics], 1, &submit_info, render_fences[frame_idx]));

        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &submit_semaphores[image_index],
            .swapchainCount = 1,
            .pSwapchains = &_swap_chain,
            .pImageIndices = &image_index,
        };
        __D2D_VULKAN_VERIFY(vkQueuePresentKHR(logi_device_ptr->queues[vk::queue_family::present], &present_info));

        frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        return {};
    }
}

namespace d2d {
    
}