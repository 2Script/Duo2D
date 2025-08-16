#pragma once
#include "Duo2D/core/basic_window.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <memory>
#include <result/verify.h>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include "Duo2D/traits/same_as.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
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
        
        //Create render pass/
        RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(logi_device));

        //Create swap chain
        RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(logi_device, phys_device, _render_pass, _surface, *this));

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

        RESULT_TRY_MOVE((*static_cast<base_type*>(this)), (make<base_type>(logi_device, phys_device, font_data_map, _render_pass)));

        //update uniform buffer
        (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);

        RESULT_TRY_MOVE(timeline_semaphore, make<vk::semaphore>(logi_device, VK_SEMAPHORE_TYPE_TIMELINE));

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
            RESULT_VERIFY((base_type::template apply_memory_changes<T>(_render_pass)))
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
            RESULT_VERIFY((base_type::template apply_attributes<T>()))
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
    template<impl::directly_renderable T>
    constexpr bool basic_window<Ts...>::has_changes() const noexcept {
        return base_type::template has_changes<T>();
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
        return base_type::template has_changes<T>() = value;
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
        std::size_t frame_idx = frame_count.value.load() % frames_in_flight;
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
        VkResult nir = vkAcquireNextImageKHR(*this->logi_device_ptr, _swap_chain, UINT64_MAX, frame_operation_semaphores[frame_operation::image_acquired][frame_idx], VK_NULL_HANDLE, &image_index);
        
        //Re-create swap chain if needed
        bool swap_chain_updated = false;
        switch(nir) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR: {
            RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(this->logi_device_ptr, this->phys_device_ptr, _render_pass, _surface, *this));
            (Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);
            swap_chain_updated = true;
            return {};
        }
        default: 
            return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(nir));
        }

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
        command_buffers[frame_idx].render_begin(_swap_chain, _render_pass, image_index);

        //TODO?: replace with functor (e.g. a generic_member_functor class)
        RESULT_VERIFY(ol::to_result((std::bind(&basic_window<Ts...>::draw<Ts>, this, frame_idx) && ...)));

        command_buffers[frame_idx].render_end();
        RESULT_VERIFY(command_buffers[frame_idx].end());

        //only 1 sempahore for now becuase current use of timeline semaphores has a possible latency:
        //TODO: semaphores need to be setup so that the current frame being rendered (and not the frame currently being submitted) needs to wait on the CPU operations
        const std::array<vk::semaphore_submit_info, 1> wait_semaphore_infos = {{
            {frame_operation_semaphores[frame_operation::image_acquired][frame_idx], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT},
            //{timeline_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, update_count.value.load()}
        }}; 
        const std::array<vk::semaphore_submit_info, 1> submit_semaphore_infos = {{
            {rendering_complete_semaphores[image_index], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT},
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
        __D2D_VULKAN_VERIFY(vkQueuePresentKHR(this->logi_device_ptr->queues[vk::queue_family::present], &present_info));

        //frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        ++frame_count.value;
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
    void basic_window<Ts...>::apply_insertion(std::string_view name, T& inserted_value) noexcept {
        insert_children<T>(name, inserted_value);
        inserted_value.on_window_insert(*this, name);
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
    template<impl::renderable_container_like T> 
    constexpr bool basic_window<Ts...>::insert_children(std::string_view name, T& container) noexcept {
        bool emplaced = false;
        for(std::size_t i = 0; i < container.size(); ++i) {
            bool formatted_key = name.length() >= container_child_prefix.length() && std::memcmp(name.data(), container_child_prefix.data(), container_child_prefix.size()) == 0;
            std::string s = formatted_key ? std::format("{}_{}", name, i) : std::format(container_child_format_key, name, std::format("_{}", i));
            auto insert_result = this->insert_or_assign<typename T::renderable_type>(s, std::move(*(container[i])));
            container.template on_window_insert_child_renderable<typename T::renderable_type>(*this, insert_result.first, i);
            emplaced = insert_result.second || emplaced;
        }
        return emplaced;
    }


    template<typename... Ts>
    template<impl::renderable_container_tuple_like T> 
    constexpr bool basic_window<Ts...>::insert_children(std::string_view name, T& tuple) noexcept {
        auto insert_child = [&, this]<std::size_t I>(std::integral_constant<std::size_t, I>) noexcept -> bool {
            bool formatted_key = name.length() >= container_child_prefix.length() && std::memcmp(name.data(), container_child_prefix.data(), container_child_prefix.size()) == 0;
            std::string s = formatted_key ? std::format("{}_{}", name, I) : std::format(container_tuple_child_format_key, name, std::format("_{}", I));
            auto insert_result = this->insert_or_assign<typename T::template container_type<I>>(s, std::move(*(tuple.template get_container<I>())));
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