#pragma once
#include "Duo2D/core/basic_window.hpp"

#include <GLFW/glfw3.h>
#include <cstring>
#include <memory>
#include <result/verify.h>
#include <type_traits>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/vulkan/device/queue_family.hpp"

namespace d2d {
    template<typename... Ts>
    result<basic_window<Ts...>> basic_window<Ts...>::create(std::string_view title, std::size_t width, std::size_t height, std::shared_ptr<vk::instance> i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        basic_window ret{glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
        __D2D_GLFW_VERIFY(ret.handle);

        //Create surface
        RESULT_TRY_MOVE(ret._surface, make<vk::surface>(ret, i));

        return ret;
    }
}

namespace d2d {
    template<typename... Ts>
    result<void> basic_window<Ts...>::initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device) noexcept {
        if(_swap_chain) 
            return {}; //return error::window_already_initialized;

        logi_device_ptr = logi_device;
        phys_device_ptr = phys_device;

        //Create command pool
        vk::command_pool c;
        RESULT_TRY_MOVE(c, make<vk::command_pool>(logi_device, phys_device));
        command_pool_ptr = std::make_shared<vk::command_pool>(std::move(c));
        
        //Create render pass/
        RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(logi_device));

        //Create swap chain
        RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(logi_device, phys_device, _render_pass, _surface, *this));

        for(std::size_t i = 0; i < impl::frames_in_flight; ++i) {
            //Create command buffers
            RESULT_TRY_MOVE(command_buffers[i], make<vk::command_buffer>(logi_device, command_pool_ptr));

            //Create fences & sempahores
            RESULT_TRY_MOVE(render_fences[i], make<vk::fence>(logi_device));
            RESULT_TRY_MOVE(frame_semaphores[semaphore_type::image_available][i], make<vk::semaphore>(logi_device));
        }
        
        //Create submit sempahores
        submit_semaphores.reserve(_swap_chain.image_count());
        for(std::size_t i = 0; i < _swap_chain.image_count(); ++i) {
            RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(logi_device), submit_semaphore);
            submit_semaphores.push_back(*std::move(submit_semaphore));
        }

        RESULT_TRY_MOVE((*static_cast<base_type*>(this)), (make<base_type>(logi_device, phys_device, _render_pass)));

        //update uniform buffer
        (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);

        return {};
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T>
    result<void> basic_window<Ts...>::apply_changes() noexcept {
        for(typename vk::impl::renderable_input_map<T>::value_type& renderable_pair : renderable_data_of<T>())
            RESULT_VERIFY(renderable_pair.second.before_changes_applied(*this));

        if constexpr(impl::directly_renderable<T>)
            RESULT_VERIFY((base_type::template apply_changes<T>(_render_pass)))
        else 
            RESULT_VERIFY(apply_changes<typename T::element_type>());

        for(typename vk::impl::renderable_input_map<T>::value_type& renderable_pair : renderable_data_of<T>())
            RESULT_VERIFY(renderable_pair.second.after_changes_applied(*this));

        return {};
    }


    template<typename... Ts>
    template<typename T>
    constexpr bool const& basic_window<Ts...>::has_changes() const noexcept {
        if constexpr(impl::directly_renderable<T>)
            return base_type::template has_changes<T>();
        else 
            return has_changes<typename T::element_type>();
    }

    template<typename... Ts>
    template<typename T>
    constexpr bool& basic_window<Ts...>::has_changes() noexcept {
        if constexpr(impl::directly_renderable<T>)
            return base_type::template has_changes<T>();
        else 
            return has_changes<typename T::element_type>();
    }
}


namespace d2d {
    template<typename... Ts>
    result<void> basic_window<Ts...>::render() noexcept {
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
            RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(logi_device_ptr, phys_device_ptr, _render_pass, _surface, *this));
            (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);
            return {};
        }
        default: 
            return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(nir));
        }

        if((this->template empty<Ts>() && ...))
            return {};


        RESULT_VERIFY(render_fences[frame_idx].reset());

        RESULT_VERIFY(command_buffers[frame_idx].reset());
        RESULT_VERIFY(command_buffers[frame_idx].render_begin(_swap_chain, _render_pass, image_index));
        
        //TODO: Simplify this
        errc error_code = error::unknown;
        auto create_descriptors = [&]<typename T>(errc& current_error_code) noexcept -> errc {
            if(current_error_code != error::unknown) return current_error_code;
            if constexpr(impl::directly_renderable<T>) RESULT_VERIFY(command_buffers[frame_idx].draw<T>(*this));
            return error::unknown;
        };
        ((error_code = create_descriptors.template operator()<Ts>(error_code)), ...);
        if(error_code != error::unknown) return error_code;

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
    template<typename... Ts>
    template<typename R>
    std::pair<typename basic_window<Ts...>::template iterator<R>, bool> basic_window<Ts...>::insert(const basic_window<Ts...>::value_type<R>& value) noexcept {
        using T = std::remove_cvref_t<R>;
        auto ins = renderable_data_of<T>().insert(value);
        if(!ins.second) return ins;
        this->template has_changes<T>() = true;
        insert_children<T>(ins.first);
        ins.first->second.template on_window_insert<T>(*this, ins.first);
        return ins;
    }

    template<typename... Ts>
    template<typename R>
    std::pair<typename basic_window<Ts...>::template iterator<R>, bool> basic_window<Ts...>::insert(basic_window<Ts...>::value_type<R>&& value) noexcept {
        using T = std::remove_cvref_t<R>;
        auto ins = renderable_data_of<T>().insert(std::move(value));
        if(!ins.second) return ins;
        this->template has_changes<T>() = true;
        insert_children<T>(ins.first);
        ins.first->second.template on_window_insert<T>(*this, ins.first);
        return ins;
    }



    template<typename... Ts>
    template<impl::constructible_from_second_type_of P>
    std::pair<typename basic_window<Ts...>::template iterator<typename std::remove_cvref_t<P>::second_type>, bool>  basic_window<Ts...>::insert(P&& value) noexcept {
        using T = typename std::remove_cvref_t<P>::second_type;
        auto ins = renderable_data_of<T>().insert(std::forward<P>(value));
        if(!ins.second) return ins;
        this->template has_changes<T>() = true;
        insert_children<T>(ins.first);
        ins.first->second.template on_window_insert<T>(*this, ins.first);
        return ins;
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T, typename... Args>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::emplace(Args&&... args) noexcept {
        auto ins = renderable_data_of<T>().emplace(std::forward<Args>(args)...);
        if(!ins.second) return ins;
        this->template has_changes<T>() = true;
        insert_children<T>(ins.first);
        ins.first->second.template on_window_insert<T>(*this, ins.first);
        return ins;
    }

    template<typename... Ts>
    template<typename T, typename S, typename... Args>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::try_emplace(S&& str, Args&&... args) noexcept 
    requires impl::not_convertible_to_iters<S, T, basic_window<Ts...>::template iterator, basic_window<Ts...>::template const_iterator> {
        auto ins = renderable_data_of<T>().try_emplace(std::forward<S>(str), std::forward<Args>(args)...);
        if(!ins.second) return ins;
        this->template has_changes<T>() = true;
        insert_children<T>(ins.first);
        ins.first->second.template on_window_insert<T>(*this, ins.first);
        return ins;
    }
}


namespace d2d {
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::iterator<T> pos) noexcept {
        return renderable_data_of<T>().erase(pos);
    }
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::const_iterator<T> pos) noexcept {
        return renderable_data_of<T>().erase(pos);
    }
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::const_iterator<T> first, basic_window<Ts...>::const_iterator<T> last) noexcept {
        return renderable_data_of<T>().erase(first, last);
    }
    template<typename... Ts>
    template<typename T>
    std::size_t basic_window<Ts...>::erase(std::string_view key) noexcept {
        return renderable_data_of<T>().erase(key);
    }
}


namespace d2d {
    template<typename... Ts>
    template<typename T>
    bool basic_window<Ts...>::empty() const noexcept {
        return renderable_data_of<T>().empty();
    }
    template<typename... Ts>
    template<typename T>
    std::size_t basic_window<Ts...>::size() const noexcept {
        return renderable_data_of<T>().size();
    }
}



namespace d2d {
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr vk::renderable_data<T, basic_window<Ts...>::frames_in_flight>& basic_window<Ts...>::renderable_data_of() noexcept {
        return base_type::template renderable_data_of<T>();
    }
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr vk::renderable_data<T, basic_window<Ts...>::frames_in_flight> const& basic_window<Ts...>::renderable_data_of() const noexcept {
        return base_type::template renderable_data_of<T>();
    }

    template<typename... Ts>
    template<impl::renderable_container_like T>
    constexpr vk::impl::renderable_input_map<T>& basic_window<Ts...>::renderable_data_of() noexcept {
        return std::get<vk::impl::renderable_input_map<T>>(renderable_container_datas);
    }
    template<typename... Ts>
    template<impl::renderable_container_like T>
    constexpr vk::impl::renderable_input_map<T> const& basic_window<Ts...>::renderable_data_of() const noexcept {
        return std::get<vk::impl::renderable_input_map<T>>(renderable_container_datas);
    }


    template<typename... Ts>
    template<impl::renderable_container_like T> 
    constexpr bool basic_window<Ts...>::insert_children(iterator<T> iter) noexcept {
        bool emplaced = false;
        for(std::size_t i = 0; i < iter->second.size(); ++i) {
            std::string s = std::format(container_child_format_key, iter->first, i);
            auto insert_result = try_emplace<typename T::element_type>(s, *(iter->second[i]));
            if(insert_result.second) iter->second.template on_window_insert_child<typename T::element_type>(*this, insert_result.first, i);
            emplaced = insert_result.second || emplaced;
        }
        return emplaced;
    }
}