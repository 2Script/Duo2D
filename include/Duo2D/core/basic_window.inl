#pragma once
#include "Duo2D/core/basic_window.hpp"

#include <GLFW/glfw3.h>
#include <atomic>
#include <climits>
#include <cstdint>
#include <cstring>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <result/verify.h>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/core/thread_pool.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/codes_map.hpp"
#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/input/interactable.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/interactable_like.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include "Duo2D/traits/same_as.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/depth_image.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/vulkan/device/queue_family.hpp"

namespace d2d {
    template<typename... Ts>
    result<basic_window<Ts...>> basic_window<Ts...>::create(std::string_view title, unsigned int width, unsigned int height, std::shared_ptr<vk::instance> i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        basic_window ret{glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
        __D2D_GLFW_VERIFY(ret.handle);

        //Set callbacks
        glfwSetKeyCallback(ret.handle.get(), kb_key_input);
        glfwSetCharCallback(ret.handle.get(), kb_text_input);
        glfwSetCursorPosCallback(ret.handle.get(), mouse_move);
        glfwSetMouseButtonCallback(ret.handle.get(), mouse_button_input);
        glfwSetScrollCallback(ret.handle.get(), mouse_scroll);

        //Create surface
        RESULT_TRY_MOVE(ret._surface, make<vk::surface>(ret, i));

        //Verify window size
        int w = 0, h = 0;
        glfwGetWindowSize(ret.handle.get(), &w, &h);
        __D2D_GLFW_VERIFY((width == static_cast<unsigned int>(w) && height == static_cast<unsigned int>(h)));
        ret._size = {width, height};

        return std::move(ret);
    }
}

namespace d2d {
    template<typename... Ts>
    result<void> basic_window<Ts...>::initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device, std::shared_ptr<impl::font_data_map> font_data_map) noexcept {
        if(_swap_chain) 
            return {}; //return error::window_already_initialized;

        //Create command pool
        vk::command_pool c;
        RESULT_TRY_MOVE(c, make<vk::command_pool>(logi_device, phys_device));
        command_pool_ptr = std::make_shared<vk::command_pool>(std::move(c));
        
        

        //Create swap chain
        RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(logi_device, phys_device, pixel_format_priority, default_color_space, present_mode_priority, _surface, *this));

        //Create render pass
        RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(logi_device, _swap_chain.format()));

        //Create depth image
        RESULT_TRY_MOVE(_depth_image, make<vk::depth_image>(logi_device, phys_device, _swap_chain.extent()));

        //Create framebuffers
        _framebuffers.resize(_swap_chain.image_count());
        for (size_t i = 0; i < _swap_chain.image_count(); i++) {
            RESULT_TRY_MOVE(_framebuffers[i], make<vk::framebuffer>(logi_device, _swap_chain.image_views()[i], _depth_image.view(), _render_pass, _swap_chain.extent()));
        }


        for(std::size_t i = 0; i < impl::frames_in_flight; ++i) {
            //Create command buffers
            RESULT_TRY_MOVE(command_buffers[i], make<vk::command_buffer>(logi_device, command_pool_ptr));

            //Create fences & sempahores
            RESULT_TRY_MOVE(render_fences[i], make<vk::fence>(logi_device));
            RESULT_TRY_MOVE(frame_operation_semaphores[frame_operation::image_acquired][i], make<vk::semaphore>(logi_device));
        }
        
        //Create submit sempahores
        rendering_complete_semaphores.reserve(_swap_chain.image_count());
        for(std::size_t i = 0; i < _swap_chain.image_count(); ++i) {
            RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(logi_device), submit_semaphore);
            rendering_complete_semaphores.push_back(*std::move(submit_semaphore));
        }

        RESULT_TRY_MOVE((*static_cast<base_tuple_type*>(this)), (make<base_tuple_type>(logi_device, phys_device, font_data_map, _render_pass)));

        //update uniform buffer
        (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);

        return {};
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T>
    result<void> basic_window<Ts...>::apply_changes() noexcept {
        RESULT_VERIFY(apply_memory_changes<T>())
        RESULT_VERIFY(apply_attributes<T>())
        has_changes<T>(false);
        return {};
    }

    template<typename... Ts>
    template<impl::when_decayed_same_as<font> T>
    result<void> basic_window<Ts...>::apply_changes() noexcept {
        for(auto const& font_path_pair : font_paths)
            RESULT_VERIFY(this->load(font_path_pair.first, font_path_pair.second.generic_string()));
        has_changes<T>(false);
        return {};
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T>
    result<void> basic_window<Ts...>::apply_memory_changes() noexcept {
        for(typename vk::impl::renderable_input_map<T>::value_type& renderable_pair : renderable_data_of<T>())
            RESULT_VERIFY(renderable_pair.second.before_changes_applied(*this));

        if constexpr(impl::directly_renderable<T>)
            RESULT_VERIFY((base_tuple_type::template apply_memory_changes<T>(_render_pass)))
        else if constexpr(impl::renderable_container_like<T>)
            RESULT_VERIFY(apply_memory_changes<typename T::renderable_type>())
        else {
            auto apply_all_memory_changes = [this]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> result<void> {
                RESULT_VERIFY(ol::to_result((std::bind(&basic_window<Ts...>::apply_memory_changes<typename T::template container_type<Is>>, this) && ...)));
                return {};
            };
            RESULT_VERIFY(apply_all_memory_changes(std::make_index_sequence<T::container_count>{}));
        }

        for(typename vk::impl::renderable_input_map<T>::value_type& renderable_pair : renderable_data_of<T>())
            RESULT_VERIFY(renderable_pair.second.after_changes_applied(*this));

        return {};
    }

    template<typename... Ts>
    template<typename T>
    result<void> basic_window<Ts...>::apply_attributes() noexcept {
        if constexpr(impl::directly_renderable<T>)
            RESULT_VERIFY((base_tuple_type::template apply_attributes<T>()))
        else if constexpr(impl::renderable_container_like<T>)
            RESULT_VERIFY(apply_attributes<typename T::renderable_type>())
        else {
            auto apply_all_attribute_changes = [this]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> result<void> {
                RESULT_VERIFY(ol::to_result((std::bind(&basic_window<Ts...>::apply_attributes<typename T::template container_type<Is>>, this) && ...)));
                return {};
            };
            RESULT_VERIFY(apply_all_attribute_changes(std::make_index_sequence<T::container_count>{}));
        }

        return {};
    }
}


namespace d2d {
    template<typename... Ts>
    result<bool> basic_window<Ts...>::verify_swap_chain(VkResult fn_result, bool even_if_suboptimal) noexcept {
        switch(fn_result) {
        case VK_SUCCESS:
            return false;
        case VK_SUBOPTIMAL_KHR:
            if(!even_if_suboptimal) return false;
            [[fallthrough]];
        case VK_ERROR_OUT_OF_DATE_KHR: {
            if(glfwWindowShouldClose(*this)) [[unlikely]] return false;
            vkDeviceWaitIdle(*this->logi_device_ptr);
            VkFormat old_format = _swap_chain.format().pixel_format.id;
            _swap_chain = {}; //delete before remaking swapchain
            RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(this->logi_device_ptr, this->phys_device_ptr, pixel_format_priority, default_color_space, present_mode_priority, _surface, *this));
            if(old_format != _swap_chain.format().pixel_format.id) {
                RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(this->logi_device_ptr, _swap_chain.format()));
                RESULT_VERIFY(this->create_descriptors(_render_pass));
            }

            RESULT_TRY_MOVE(_depth_image, make<vk::depth_image>(this->logi_device_ptr, this->phys_device_ptr, _swap_chain.extent()));
            for(std::size_t i = 0; i < _swap_chain.image_count(); ++i)
                RESULT_TRY_MOVE(_framebuffers[i], make<vk::framebuffer>(this->logi_device_ptr, _swap_chain.image_views()[i], _depth_image.view(), _render_pass, _swap_chain.extent()));
            (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);
            return true;
        }
        default: 
            return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(fn_result));
        }
    }
}



namespace d2d {
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr bool basic_window<Ts...>::has_changes() const noexcept {
        return base_tuple_type::template has_changes<T>();
    }

    template<typename... Ts>
    template<impl::renderable_container_like T>
    constexpr bool basic_window<Ts...>::has_changes() const noexcept {
        return this->has_changes<typename T::renderable_type>();
    }

    template<typename... Ts>
    template<impl::renderable_container_tuple_like T>
    constexpr bool basic_window<Ts...>::has_changes() const noexcept {
        return [this]<std::size_t... I>(std::index_sequence<I...>) noexcept -> bool {
            return (this->has_changes<typename T::template container_type<I>>() || ...);
        }(std::make_index_sequence<T::container_count>{});
    }

    template<typename... Ts>
    template<impl::when_decayed_same_as<font> T>
    constexpr bool basic_window<Ts...>::has_changes() const noexcept {
        return font_paths_outdated;
    }

    
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr bool basic_window<Ts...>::has_changes(bool value) noexcept {
        return base_tuple_type::template has_changes<T>() = value;
    }

    template<typename... Ts>
    template<impl::renderable_container_like T>
    constexpr bool basic_window<Ts...>::has_changes(bool value) noexcept {
        return this->has_changes<typename T::renderable_type>(value);
    }

    template<typename... Ts>
    template<impl::renderable_container_tuple_like T>
    constexpr bool basic_window<Ts...>::has_changes(bool value) noexcept {
        return [this]<std::size_t... I>(bool val, std::index_sequence<I...>) noexcept -> bool {
            return (this->has_changes<typename T::template container_type<I>>(val) || ...);
        }(value, std::make_index_sequence<T::container_count>{});
    }

    template<typename... Ts>
    template<impl::when_decayed_same_as<font> T>
    constexpr bool basic_window<Ts...>::has_changes(bool value) noexcept {
        return font_paths_outdated = value;
    }
}


namespace d2d {
    template<typename... Ts>
    result<void> basic_window<Ts...>::render() noexcept {
        std::size_t frame_idx = frame_count.load() % frames_in_flight;
        //wait for rendering to finish last frame
        RESULT_VERIFY(render_fences[frame_idx].wait());

        //std::size_t wait_value = update_count.value.load();
        //VkSemaphoreWaitInfo wait_info {
        //    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        //    .pNext = nullptr,
        //    .flags = 0,
        //    .semaphoreCount = 1,
        //    .pSemaphores = &test_semaphore,
        //    .pValues = &wait_value,
        //};
        //vkWaitSemaphores(logical_device(), &wait_info, UINT64_MAX);

        uint32_t image_index;
        RESULT_TRY_COPY_UNSCOPED(bool swap_chain_updated, verify_swap_chain(
            vkAcquireNextImageKHR(*this->logi_device_ptr, _swap_chain, UINT64_MAX, frame_operation_semaphores[frame_operation::image_acquired][frame_idx], VK_NULL_HANDLE, &image_index), false
        ), scu);

        if((this->template empty<Ts>() && ...))
            return {};


        RESULT_VERIFY(render_fences[frame_idx].reset());
        RESULT_VERIFY(command_buffers[frame_idx].reset());

        RESULT_VERIFY(command_buffers[frame_idx].begin(false));
        VkMemoryBarrier2 global_barrier {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
            .srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
        };
        command_buffers[frame_idx].pipeline_barrier(std::span<const VkMemoryBarrier2, 1>{&global_barrier, 1}, {swap_chain_updated ? &this->uniform_buff_barrier : nullptr, static_cast<std::size_t>(swap_chain_updated)}, {});
        command_buffers[frame_idx].render_begin(_swap_chain, _render_pass, _framebuffers, image_index);

        //TODO?: replace with functor (e.g. a generic_member_functor class)
        std::unique_lock<std::mutex> draw_cmd_buff_lock(this->draw_cmd_buff_mutexes[frame_idx]);
        RESULT_VERIFY(ol::to_result((std::bind(&basic_window<Ts...>::draw<Ts>, this, frame_idx) && ...)));
        std::size_t draw_cmd_idx = this->draw_cmd_update_count++;//this->draw_cmd_update_count.fetch_add(1, std::memory_order::relaxed);
        draw_cmd_buff_lock.unlock();

        command_buffers[frame_idx].render_end();
        RESULT_VERIFY(command_buffers[frame_idx].end());

        //only 1 sempahore for now becuase current use of timeline semaphores has a possible latency:
        //TODO: semaphores need to be setup so that the current frame being rendered (and not the frame currently being submitted) needs to wait on the CPU operations
        const std::array<vk::semaphore_submit_info, 2> wait_semaphore_infos = {{
            {frame_operation_semaphores[frame_operation::image_acquired][frame_idx], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT},
            {this->draw_cmd_update_semaphore, VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, draw_cmd_idx}
        }}; 
        const std::array<vk::semaphore_submit_info, 2> submit_semaphore_infos = {{
            {rendering_complete_semaphores[image_index], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT},
            {this->draw_cmd_update_semaphore, VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, draw_cmd_idx + 1}
        }}; 
        RESULT_VERIFY(command_buffers[frame_idx].submit(vk::queue_family::graphics, wait_semaphore_infos, submit_semaphore_infos, render_fences[frame_idx]));


        
        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &rendering_complete_semaphores[image_index],
            .swapchainCount = 1,
            .pSwapchains = &_swap_chain,
            .pImageIndices = &image_index,
        };
        RESULT_VERIFY(verify_swap_chain(vkQueuePresentKHR(this->logi_device_ptr->queues[vk::queue_family::present], &present_info), true));

        //frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        ++frame_count;
        return {};
    }


    template<typename... Ts>
    template<typename T>
    result<void> basic_window<Ts...>::draw(std::size_t frame_idx) const noexcept {
        if constexpr(impl::directly_renderable<T>)
            return command_buffers[frame_idx].draw<T>(*this);
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
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }

    template<typename... Ts>
    template<typename R>
    std::pair<typename basic_window<Ts...>::template iterator<R>, bool> basic_window<Ts...>::insert(basic_window<Ts...>::value_type<R>&& value) noexcept {
        using T = std::remove_cvref_t<R>;
        auto ins = renderable_data_of<T>().insert(std::move(value));
        if(!ins.second) return ins;
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }


    template<typename... Ts>
    template<impl::constructible_from_second_type_of P>
    std::pair<typename basic_window<Ts...>::template iterator<typename std::remove_cvref_t<P>::second_type>, bool>  basic_window<Ts...>::insert(P&& value) noexcept {
        using T = typename std::remove_cvref_t<P>::second_type;
        auto ins = renderable_data_of<T>().insert(std::forward<P>(value));
        if(!ins.second) return ins;
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T, typename V>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::insert_or_assign(const key_type<T>& key, V&& value) noexcept {
        auto ins = renderable_data_of<T>().insert_or_assign(key, std::forward<V>(value));
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }

    template<typename... Ts>
    template<typename T, typename V>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::insert_or_assign(key_type<T>&& key, V&& value) noexcept {
        auto ins = renderable_data_of<T>().insert_or_assign(std::move(key), std::forward<V>(value));
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T, typename... Args>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::emplace(Args&&... args) noexcept {
        auto ins = renderable_data_of<T>().emplace(std::forward<Args>(args)...);
        if(!ins.second) return ins;
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }

    template<typename... Ts>
    template<typename T, typename S, typename... Args>
    std::pair<typename basic_window<Ts...>::template iterator<T>, bool> basic_window<Ts...>::try_emplace(S&& str, Args&&... args) noexcept 
    requires impl::not_convertible_to_iters<S, T, basic_window<Ts...>::template iterator, basic_window<Ts...>::template const_iterator> {
        auto ins = renderable_data_of<T>().try_emplace(std::forward<S>(str), std::forward<Args>(args)...);
        if(!ins.second) return ins;
        this->template has_changes<T>(true);
        if constexpr(!impl::when_decayed_same_as<T, font>)
            apply_insertion<T>(ins.first->first, ins.first->second);
        return ins;
    }
}

namespace d2d {
    template<typename... Ts>
    template<typename T>
    void basic_window<Ts...>::apply_insertion(key_type<T> key, T& inserted_value) noexcept {
        insert_children<T>(key, inserted_value);
        if constexpr(impl::interactable_like<T>) interactables_of<T>().insert_or_assign(key, std::make_pair(std::ref(inserted_value), false));//interactable_ptrs.push_back(static_cast<interactable*>(&inserted_value));
        inserted_value.template on_window_insert<T>(*this, key);
    }
}


namespace d2d {
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::iterator<T> pos) noexcept {
        if constexpr(impl::interactable_like<T>) {
            interactables_of<T>().erase(pos->first);
            ////TODO: improve this after using a vector for renderable inputs
            //auto it = std::remove(interactable_ptrs.begin(), interactable_ptrs.end(), static_cast<interactable*>(&pos->second));
            //std::size_t erase_idx = std::distance(interactable_ptrs.begin(), it);
            //focus_idx.compare_exchange_strong(erase_idx, static_cast<std::size_t>(-1), std::memory_order_relaxed);
            //interactable_ptrs.erase(it, interactable_ptrs.end());
        }

        return renderable_data_of<T>().erase(pos);
    }
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::const_iterator<T> pos) noexcept {
        if constexpr(impl::interactable_like<T>){
            interactables_of<T>().erase(pos->first);
            ////TODO: improve this after using a vector for renderable inputs
            //auto it = std::remove(interactable_ptrs.begin(), interactable_ptrs.end(), static_cast<interactable*>(&pos->second));
            //std::size_t erase_idx = std::distance(interactable_ptrs.begin(), it);
            //focus_idx.compare_exchange_strong(erase_idx, static_cast<std::size_t>(-1), std::memory_order_relaxed);
            //interactable_ptrs.erase(it, interactable_ptrs.end());
        }

        return renderable_data_of<T>().erase(pos);
    }
    template<typename... Ts>
    template<typename T>
    basic_window<Ts...>::iterator<T> basic_window<Ts...>::erase(basic_window<Ts...>::const_iterator<T> first, basic_window<Ts...>::const_iterator<T> last) noexcept {
        if constexpr(impl::interactable_like<T>) {
            for(auto map_it = first; map_it != last; ++map_it){
                interactables_of<T>().erase(map_it->first);
                ////TODO: improve this after using a vector for renderable inputs
                //auto it = std::remove(interactable_ptrs.begin(), interactable_ptrs.end(), static_cast<interactable*>(&map_it->second));
                //std::size_t erase_idx = std::distance(interactable_ptrs.begin(), it);
                //focus_idx.compare_exchange_strong(erase_idx, static_cast<std::size_t>(-1), std::memory_order_relaxed);
                //interactable_ptrs.erase(it, interactable_ptrs.end());
            }
        }
        return renderable_data_of<T>().erase(first, last);
    }
    template<typename... Ts>
    template<typename T>
    std::size_t basic_window<Ts...>::erase(key_type<T> const& key) noexcept {
        if constexpr(impl::interactable_like<T>){
            interactables_of<T>().erase(key);
            ////TODO: improve this after using a vector for renderable inputs
            //auto it = std::remove(interactable_ptrs.begin(), interactable_ptrs.end(), static_cast<interactable*>(&renderable_data_of<T>().find(key)->second));
            //std::size_t erase_idx = std::distance(interactable_ptrs.begin(), it);
            //focus_idx.compare_exchange_strong(erase_idx, static_cast<std::size_t>(-1), std::memory_order_relaxed);
            //interactable_ptrs.erase(it, interactable_ptrs.end());
        }
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
    template<typename T>
    result<void> basic_window<Ts...>::set_hidden(key_type<T> const& key, bool value) noexcept {
        if constexpr(impl::directly_renderable<T>)
            RESULT_VERIFY((base_tuple_type::template set_hidden<T>(key, value)))
        else if constexpr(impl::renderable_container_like<T>) {
            auto it = renderable_data_of<T>().find(key);
            if(it == renderable_data_of<T>().end()) return errc::element_not_found;

            for(std::size_t i = 0; i < it->second.size(); ++i)
                RESULT_VERIFY(set_hidden<typename T::renderable_type>(it->second.child_keys[i], value))
        }
        else {
            auto it = renderable_data_of<T>().find(key);
            if(it == renderable_data_of<T>().end()) return errc::element_not_found;

            auto set_all_hidden = [this]<std::size_t... Is>(T const& tuple, bool v, std::index_sequence<Is...>) noexcept -> result<void> {
                RESULT_VERIFY(ol::to_result((std::bind(&basic_window<Ts...>::set_hidden<typename T::template container_type<Is>>, this, tuple.template get_container<Is>()->self_key, v) && ...)));
                return {};
            };
            RESULT_VERIFY(set_all_hidden(it->second, value, std::make_index_sequence<T::container_count>{}));
        }
        return {};
    }
}



namespace d2d {
    template<typename... Ts>
    void basic_window<Ts...>::process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept {
        auto window_info_it = input::impl::glfw_window_map().find(window_ptr);
        if(window_info_it == input::impl::glfw_window_map().end()) [[unlikely]] return;
        basic_window<Ts...>* win_ptr = static_cast<basic_window<Ts...>*>(window_info_it->second.window_ptr);

        std::unique_lock<std::mutex> current_combo_lock(window_info_it->second.combo_mutex);
        window_info_it->second.current_combo.main_input() = code;
        window_info_it->second.current_combo.set(code, false);
        input::combination combo = (win_ptr->input_modifier_flags()[code] & input::modifier_flags::no_modifiers_allowed) ? input::combination{{}, code} : window_info_it->second.current_combo;
        window_info_it->second.current_combo.set(code, pressed);
        current_combo_lock.unlock();
        
        input::binding_map const& bind_map = pressed ? win_ptr->input_active_bindings() : win_ptr->input_inactive_bindings();
        auto event_set_it = bind_map.find(combo);
        if(event_set_it != bind_map.end()) goto invoke_event;

        event_set_it = bind_map.find(input::combination{{d2d::input::generic_code::any}, code});
        if(event_set_it != bind_map.end()) goto invoke_event;

        combo.main_input() = d2d::input::generic_code::any;
        event_set_it = bind_map.find(std::move(combo));
        if(event_set_it != bind_map.end()) goto invoke_event;

        event_set_it = bind_map.find(input::combination{{d2d::input::generic_code::any}, d2d::input::generic_code::any});
        if(event_set_it != bind_map.end()) goto invoke_event;

        return;
        
    invoke_event:
        input::category_flags_t category_flags = win_ptr->current_input_categories() & event_set_it->second.applicable_categories;
        //const unsigned long long category_flags = category_bitset.to_ullong();
        while(category_flags.any()) {
            input::category_id_t category_id = input::max_category_id - std::countl_zero(category_flags.to_ullong());
            input::event_id_t event_id = event_set_it->second.event_ids[category_id];
            auto event_fn_it = win_ptr->event_functions().find(input::categorized_event_t{event_id, category_id});
            if(event_fn_it != win_ptr->event_functions().end()) {
                std::invoke(event_fn_it->second, win_ptr, combo, pressed, input::categorized_event_t{event_id, category_id}, mouse_aux_data, glfwGetWindowUserPointer(window_ptr));
                //return;
            }
            category_flags.reset(category_id);
        }
    }


    template<typename... Ts>
    void basic_window<Ts...>::kb_key_input(GLFWwindow* window_ptr, int key, int, int action, int) noexcept {
        if(key == GLFW_KEY_UNKNOWN) return;
        switch(action) {
        case GLFW_RELEASE:
        case GLFW_PRESS:
            break;
        default:
            return;
        }

        thread_pool().detach_task(std::bind(process_input, window_ptr, input::codes_map[key], static_cast<bool>(action), input::mouse_aux_t{}));
        //return process_input(window_ptr, input::codes_map[key], static_cast<bool>(action), input::mouse_aux_t{});
    }

    template<typename... Ts>
    void basic_window<Ts...>::kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept {
        auto window_info_it = input::impl::glfw_window_map().find(window_ptr);
        if(window_info_it == input::impl::glfw_window_map().end()) [[unlikely]] return;
        basic_window<Ts...>* win_ptr = static_cast<basic_window<Ts...>*>(window_info_it->second.window_ptr);

        std::function<input::text_event_function> const& text_input_fn = win_ptr->text_input_function();
        if(!text_input_fn) return;
        thread_pool().detach_task(std::bind(text_input_fn, win_ptr, codepoint));
        //std::invoke(text_input_fn, win_ptr, codepoint);
    }

    template<typename... Ts>
    void basic_window<Ts...>::mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, true, input::mouse_aux_t{pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, false, input::mouse_aux_t{pt2d{x, y}}));
        //return process_input(window_ptr, input::mouse_code::move, std::nullopt, pt2d{x, y});
    }

    template<typename... Ts>
    void basic_window<Ts...>::mouse_button_input(GLFWwindow* window_ptr, int button, int action, int) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{}));
        //return process_input(window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{});
    }

    template<typename... Ts>
    void basic_window<Ts...>::mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, true, input::mouse_aux_t{-pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, false, input::mouse_aux_t{-pt2d{x, y}}));
    }
}



namespace d2d {
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr vk::renderable_data<T, basic_window<Ts...>::frames_in_flight>& basic_window<Ts...>::renderable_data_of() noexcept {
        return base_tuple_type::template renderable_data_of<T>();
    }
    template<typename... Ts>
    template<impl::directly_renderable T>
    constexpr vk::renderable_data<T, basic_window<Ts...>::frames_in_flight> const& basic_window<Ts...>::renderable_data_of() const noexcept {
        return base_tuple_type::template renderable_data_of<T>();
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
    template<impl::renderable_container_tuple_like T>
    constexpr vk::impl::renderable_input_map<T>& basic_window<Ts...>::renderable_data_of() noexcept {
        return std::get<vk::impl::renderable_input_map<T>>(renderable_container_datas);
    }
    template<typename... Ts>
    template<impl::renderable_container_tuple_like T>
    constexpr vk::impl::renderable_input_map<T> const& basic_window<Ts...>::renderable_data_of() const noexcept {
        return std::get<vk::impl::renderable_input_map<T>>(renderable_container_datas);
    }

    template<typename... Ts>
    template<impl::when_decayed_same_as<font> T>
    constexpr impl::font_path_map& basic_window<Ts...>::renderable_data_of() noexcept {
        return font_paths;
    }
    template<typename... Ts>
    template<impl::when_decayed_same_as<font> T>
    constexpr impl::font_path_map const& basic_window<Ts...>::renderable_data_of() const noexcept {
        return font_paths;
    }


    template<typename... Ts>
    template<impl::interactable_like T>
    constexpr vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>>& basic_window<Ts...>::interactables_of() noexcept {
        return std::get<vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>>>(interactable_refs);
    }
    template<typename... Ts>
    template<impl::interactable_like T>
    constexpr vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>> const& basic_window<Ts...>::interactables_of() const noexcept {
        return std::get<vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>>>(interactable_refs);
    }


    template<typename... Ts>
    template<impl::renderable_container_like T> 
    constexpr bool basic_window<Ts...>::insert_children(key_type<T> key, T& container) noexcept {
        bool emplaced = false;
        key_type<T> k = (key & 0xFF00'0000'0000'0000) + (key << (sizeof(std::uint8_t) * CHAR_BIT));
        for(std::size_t i = 0; i < container.size(); ++i) {
            auto insert_result = this->insert_or_assign<typename T::renderable_type>(k + i, std::move(*(container[i])));
            container.template on_window_insert_child_renderable<typename T::renderable_type>(*this, insert_result.first, i);
            emplaced = insert_result.second || emplaced;
        }
        return emplaced;
    }


    template<typename... Ts>
    template<impl::renderable_container_tuple_like T> 
    constexpr bool basic_window<Ts...>::insert_children(key_type<T> key, T& tuple) noexcept {
        key_type<T> k = (key & 0xFF00'0000'0000'0000) + (key << (sizeof(std::uint8_t) * CHAR_BIT));
        auto insert_child = [&, this]<std::size_t I>(std::integral_constant<std::size_t, I>) noexcept -> bool {
            auto insert_result = this->insert_or_assign<typename T::template container_type<I>>(k + I, std::move(*(tuple.template get_container<I>())));
            tuple.template on_window_insert_child_container<I>(*this, insert_result.first);
            return insert_result.second;
        };

        bool updated = false;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) noexcept {
            ((updated = (insert_child(std::integral_constant<std::size_t, Is>{}) || updated)), ...);
        }(std::make_index_sequence<T::container_count>{});
        return updated;
    }
}