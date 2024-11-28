#include "Duo2D/graphics/pipeline/window.hpp"

#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <numbers>
#include <vulkan/vulkan_core.h>

#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/descriptor_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/image_view.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/shader_module.hpp"
#include "Duo2D/graphics/pipeline/surface.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"
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

        //Create descriptor set layout
        __D2D_TRY_MAKE(_descriptor_set_layout, make<descriptor_set_layout>(logi_device), dsl);

        //Create pipeline
        __D2D_TRY_MAKE(_pipeline_layout, make<pipeline_layout>(logi_device, _descriptor_set_layout), pl);
        __D2D_TRY_MAKE(_pipeline, make<pipeline>(logi_device, _render_pass, _pipeline_layout), p);

        //Create command pool
        __D2D_TRY_MAKE(_command_pool, make<command_pool>(logi_device, phys_device), cp);

        for(std::size_t i = 0; i < frames_in_flight; ++i) {
            //Create command buffers
            __D2D_TRY_MAKE(command_buffers[i], make<command_buffer>(logi_device, _command_pool), cb);

            //Create fences & sempahores
            __D2D_TRY_MAKE(render_fences[i], make<fence>(logi_device), f);
            __D2D_TRY_MAKE(image_available_semaphores[i], make<semaphore>(logi_device), ia);
            __D2D_TRY_MAKE(cmd_buffer_finished_semaphores[i], make<semaphore>(logi_device), cbf);

            //Create uniform buffers
            __D2D_TRY_MAKE(uniform_buffers[i], make<buffer>(logi_device, sizeof(vertex2::uniform_type), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), ub);
            __D2D_TRY_MAKE(uniform_buffer_memories[i], make<device_memory>(logi_device, phys_device, uniform_buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), ubm);
            vkMapMemory(logi_device, uniform_buffer_memories[i], 0, sizeof(vertex2::uniform_type), 0, &uniform_buffer_maps[i]);
        }

        //Create descriptor set and pool
        __D2D_TRY_MAKE(_descriptor_pool, make<descriptor_pool<frames_in_flight>>(logi_device), dp);
        __D2D_TRY_MAKE(_descriptor_set, make<descriptor_set<frames_in_flight>>(logi_device, _descriptor_pool, _descriptor_set_layout, uniform_buffers), ds);


        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    result<void> window::render() noexcept {
        //wait for rendering to finish last frame
        render_fences[frame_idx].wait();

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

        if(vk_vertex_buffers.empty())
            return result<void>{std::in_place_type<void>};

        
        //update uniform buffer
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        constexpr static float deg_to_rad = std::numbers::pi_v<float> / 180;
        vertex2::uniform_type ubo {
            .model = vk_mat4::rotating(90 * time * deg_to_rad, axis::z, std::sinf, std::cosf),
            .view = vk_mat4::looking_at({2,0,2}, {0,0,0}, axis::z, std::sqrtf),
            .proj = vk_mat4::perspective(45 * deg_to_rad, _swap_chain.extent.width(), _swap_chain.extent.height(), std::tanf, 0.1f),
        };
        std::memcpy(uniform_buffer_maps[frame_idx], &ubo, sizeof(ubo));


        render_fences[frame_idx].reset();

        command_buffers[frame_idx].reset();
        command_buffers[frame_idx].record(*this, vk_vertex_buffers, index_buffer, &(_descriptor_set.data()[frame_idx]), buffer_offsets, index_count, image_index);


        constexpr static std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_available_semaphores[frame_idx],
            .pWaitDstStageMask = wait_stages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffers[frame_idx],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &cmd_buffer_finished_semaphores[frame_idx],
        };
        __D2D_VULKAN_VERIFY(vkQueueSubmit(logi_device_ptr->queues[queue_family::graphics], 1, &submit_info, render_fences[frame_idx]));

        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &cmd_buffer_finished_semaphores[frame_idx],
            .swapchainCount = 1,
            .pSwapchains = &_swap_chain,
            .pImageIndices = &image_index,
        };
        __D2D_VULKAN_VERIFY(vkQueuePresentKHR(logi_device_ptr->queues[queue_family::present], &present_info));

        frame_idx = (frame_idx + 1) % frames_in_flight;
        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    
}