#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/prim/glyph.hpp"
#include "Duo2D/graphics/prim/debug_rect.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"

#if __cpp_lib_constexpr_memory >= 202202L
#define __D2D_CONSTEXPR_UNIQUE_PTR constexpr
#else 
#define __D2D_CONSTEXPR_UNIQUE_PTR
#endif

namespace d2d {
    struct window {
        static result<window> create(std::string_view title, std::size_t width, std::size_t height, const vk::instance& i) noexcept;

        window() noexcept : window(nullptr) {}
        result<void> initialize(vk::logical_device& logi_device, vk::physical_device& phys_device) noexcept;

        result<void> render() noexcept;

    public:
        template<typename R> using value_type = std::pair<std::string_view, R>;
        template<typename R> using iterator = typename std::unordered_map<std::string_view, R>::iterator;
        template<typename R> using const_iterator = typename std::unordered_map<std::string_view, R>::const_iterator;

        template<typename R> requires ::d2d::impl::renderable_like<std::remove_cvref_t<R>>
        std::pair<iterator<R>, bool> insert(const value_type<R>& value) noexcept;
        template<typename R> requires ::d2d::impl::renderable_like<std::remove_cvref_t<R>>
        std::pair<iterator<R>, bool> insert(value_type<R>&& value) noexcept;
        template<typename P> requires std::is_constructible_v<value_type<typename std::remove_cvref_t<P>::second_type>, P&&>
        std::pair<iterator<typename std::remove_cvref_t<P>::second_type>, bool> insert(P&& value) noexcept;

        template<typename T, typename... Args>
        std::pair<iterator<T>, bool> emplace(Args&&... args) noexcept;
        template<typename T, typename... Args>
        std::pair<iterator<T>, bool> try_emplace(std::string_view str, Args&&... args) noexcept;
        template<typename T, typename S, typename... Args> requires std::is_constructible_v<std::string, S&&>
        std::pair<iterator<T>, bool> try_emplace(S&& str, Args&&... args) noexcept;

        template<typename T>
        iterator<T> erase(iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> first, const_iterator<T> last) noexcept;
        template<typename T>
        std::size_t erase(std::string_view key) noexcept;

        template<typename T>
        iterator<T> end() noexcept;
        template<typename T>
        const_iterator<T> end() const noexcept;
        template<typename T>
        const_iterator<T> cend() const noexcept;

        template<typename T>
        iterator<T> begin() noexcept;
        template<typename T>
        const_iterator<T> begin() const noexcept;
        template<typename T>
        const_iterator<T> cbegin() const noexcept;

        template<typename T>
        bool empty() const noexcept;
        template<typename T>
        std::size_t size() const noexcept;

    public:
        template<typename T>
        result<void> apply() noexcept;

    public:
        __D2D_CONSTEXPR_UNIQUE_PTR operator GLFWwindow*() const noexcept { return handle.get(); }
        __D2D_CONSTEXPR_UNIQUE_PTR explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : 
            handle(w, {}), logi_device_ptr(nullptr), phys_device_ptr(nullptr),
            _surface(), _swap_chain(), _render_pass(),
            frame_idx(0), _command_pool(), command_buffers{}, render_fences{}, frame_semaphores{}, submit_semaphores(),
            data() {}
        friend vk::physical_device;
        
    private:
        constexpr static std::size_t frames_in_flight = 2;

        std::unique_ptr<GLFWwindow, generic_functor<glfwDestroyWindow>> handle;
        vk::logical_device* logi_device_ptr;
        vk::physical_device* phys_device_ptr;
        //Decleration order matters: swap_chain MUST be destroyed before surface
        vk::surface _surface;
        vk::swap_chain _swap_chain;
        vk::render_pass _render_pass;

        std::size_t frame_idx;
        vk::command_pool _command_pool;
        std::array<vk::command_buffer, frames_in_flight> command_buffers;
        std::array<vk::fence, frames_in_flight> render_fences;
        struct semaphore_type { enum { image_available, /*cmd_buffer_finished,*/ num_semaphore_types }; };
        std::array<std::array<vk::semaphore, frames_in_flight>, semaphore_type::num_semaphore_types> frame_semaphores;
        std::vector<vk::semaphore> submit_semaphores;

        vk::renderable_tuple<frames_in_flight, styled_rect, debug_rect, clone_rect, glyph> data;

        //constexpr static std::size_t x = decltype(data)::template static_offsets<clone_rect>()[buffer_data_type::index];
        //constexpr static std::size_t y = renderable_data<clone_rect, 2>::static_index_data_bytes.size();
    };
}

#include "Duo2D/core/window.inl"