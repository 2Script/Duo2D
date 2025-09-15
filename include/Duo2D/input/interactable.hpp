#pragma once
#include <atomic>
#include <compare>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Duo2D/arith/point.hpp"
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/input/map_types.hpp"

namespace d2d {
    template<typename... Ts>
    struct basic_window;
}

namespace d2d::input {
    using mouse_pos_t = std::optional<pt2d>;
}

namespace d2d {
    struct interactable {
        //constexpr pt2d position() const noexcept;
        //constexpr bool contains(pt2d) const noexcept;
    public:
        template<typename... Ts>
        constexpr void on_press(basic_window<Ts...>&, input::combination, input::mouse_pos_t) noexcept {}
        template<typename... Ts>
        constexpr void on_release(basic_window<Ts...>&, input::combination, input::mouse_pos_t) noexcept {}
        template<typename... Ts>
        constexpr void on_hover(basic_window<Ts...>&, pt2d) noexcept {}
    public:
        template<typename... Ts>
        constexpr void on_gain_focus(basic_window<Ts...>&, input::combination, input::mouse_pos_t) noexcept {}
        template<typename... Ts>
        constexpr void on_lose_focus(basic_window<Ts...>&, input::combination, input::mouse_pos_t) noexcept {}
    };
}


//TEMP system for default inputs
namespace d2d::input::defaults {
    struct interactable {
        template<typename WindowT>
        inline static std::array<typename event_fns_map::value_type, 3> event_fns() {
            static std::array<typename event_fns_map::value_type, 3> ret{{
                {categorized_event_t{event::mouse_press, category::ui}, [](void* win, d2d::input::combination combo, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
                    WindowT* win_ptr = static_cast<WindowT*>(win);
                    pt2d mouse_pos;
                    glfwGetCursorPos(*win_ptr, &mouse_pos.x(), &mouse_pos.y());

                    [=]<std::size_t... Is>(std::index_sequence<Is...>){
                        std::size_t old_key = 0, new_key = 0;
                        {
                        auto check_ref = [=, &old_key, &new_key]<std::size_t I>(std::integral_constant<std::size_t, I>) -> int {
                            auto& interactable_refs_map = std::get<I>(win_ptr->interactable_refs);
                            for(auto it = interactable_refs_map.begin(); it != interactable_refs_map.end(); ++it) {
                                if(it->second.first.get().contains(mouse_pos)) {
                                    it->second.first.get().on_press(*win_ptr, combo, mouse_pos);
                                    it->second.second = true;
                                    new_key = it->first;
                                    old_key = win_ptr->focus_key.exchange(new_key, std::memory_order_relaxed);
                                    if(old_key != new_key) it->second.first.get().on_gain_focus(*win_ptr, combo, mouse_pos);
                                    return true;
                                }
                            }
                            return false;
                        };
                        bool result = (check_ref(std::integral_constant<std::size_t, Is>{}) || ...);
                        if(!result) { 
                            new_key = static_cast<std::uint64_t>(-1);
                            old_key = win_ptr->focus_key.exchange(new_key, std::memory_order_relaxed);
                        }
                        }

                        {
                        if(old_key == new_key) return;
                        auto switch_focus = [=]<std::size_t I>(std::integral_constant<std::size_t, I>) -> bool {
                            auto& interactable_refs_map = std::get<I>(win_ptr->interactable_refs);
                            auto it = interactable_refs_map.find(old_key);
                            if(it != interactable_refs_map.end()) {
                                it->second.first.get().on_lose_focus(*win_ptr, combo, mouse_pos);
                                return true;
                            }
                            return false;
                        };
                        static_cast<void>((switch_focus(std::integral_constant<std::size_t, Is>{}) || ...));
                        }
                    }(std::make_index_sequence<std::tuple_size_v<decltype(win_ptr->interactable_refs)>>{});
                }},
                
                {categorized_event_t{event::mouse_release, category::ui}, [](void* win, d2d::input::combination combo, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
                    WindowT* win_ptr = static_cast<WindowT*>(win);
                    pt2d mouse_pos;
                    glfwGetCursorPos(*win_ptr, &mouse_pos.x(), &mouse_pos.y());

                    //TODO: shared_mutex
                    [=]<std::size_t... Is>(std::index_sequence<Is...>){
                        auto check_ref = [=]<std::size_t I>(std::integral_constant<std::size_t, I>) -> bool {
                            auto& interactable_refs_map = std::get<I>(win_ptr->interactable_refs);
                            for(auto it = interactable_refs_map.begin(); it != interactable_refs_map.end(); ++it) {
                                if(it->second.second) {
                                    it->second.first.get().on_release(*win_ptr, combo, mouse_pos);
                                    it->second.second = false;
                                    return true;
                                }
                            }
                            return false;
                        };
                        static_cast<void>((check_ref(std::integral_constant<std::size_t, Is>{}) || ...));
                    }(std::make_index_sequence<std::tuple_size_v<decltype(win_ptr->interactable_refs)>>{});
                }},

                
                {categorized_event_t{event::mouse_move, category::ui}, [](void* win, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t mouse_pos, void*){
                    WindowT* win_ptr = static_cast<WindowT*>(win);
                    //TODO: shared_mutex
                    [=]<std::size_t... Is>(std::index_sequence<Is...>){
                        auto check_ref = [=]<std::size_t I>(std::integral_constant<std::size_t, I>) -> bool {
                            auto& interactable_refs_map = std::get<I>(win_ptr->interactable_refs);
                            for(auto it = interactable_refs_map.begin(); it != interactable_refs_map.end(); ++it) {
                                if(it->second.first.get().contains(*mouse_pos)) {
                                    it->second.first.get().on_hover(*win_ptr, *mouse_pos);
                                    return true;
                                }
                            }
                            return false;
                        };
                        static_cast<void>((check_ref(std::integral_constant<std::size_t, Is>{}) || ...));
                    }(std::make_index_sequence<std::tuple_size_v<decltype(win_ptr->interactable_refs)>>{});
                }},

                //TODO:
                //{categorized_event_t{event::next_focus, category::ui}, [](void* win, d2d::input::combination combo, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t mouse_pos, void*){
                //    WindowT* win_ptr = static_cast<WindowT*>(win);
                //    //TODO: shared_mutex
                //    const std::size_t num_interactables = win_ptr->interactable_ptrs.size();
                //    for(std::size_t i = 0; i < num_interactables; ++i)
                //        if(win_ptr->interactable_ptrs[i]->contains(*mouse_pos)) {
                //            const std::size_t old_idx = win_ptr->focus_idx.fetch_add(1, std::memory_order_relaxed);
                //            if(old_idx != static_cast<std::size_t>(-1))
                //                win_ptr->interactable_ptrs[old_idx % num_interactables]->on_lose_focus(*win_ptr, combo, mouse_pos);
                //            win_ptr->interactable_ptrs[(old_idx + 1) % num_interactables]->on_gain_focus(*win_ptr, combo, mouse_pos);
                //            return;
                //        }
                //}},
            }};
            return ret;
        }

        template<typename... Ts>
        inline static std::array<typename binding_map::value_type, 3> press_bindings() {
            static std::array<typename binding_map::value_type, 3> ret{{
                {d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::mouse_code::move}, d2d::input::event_set{{0, event::mouse_move}, {0b1 << category::ui}}},
                {d2d::input::combination{{}, d2d::input::mouse_code::button_1}, d2d::input::event_set{{0, event::mouse_press}, {0b1 << category::ui}}},
                {d2d::input::combination{{}, d2d::input::key_code::kb_tab}, d2d::input::event_set{{0, event::next_focus}, {0b1 << category::ui}}},
            }};
            return ret;
        }
        template<typename... Ts>
        inline static std::array<typename binding_map::value_type, 1> release_bindings() {
            static std::array<typename binding_map::value_type, 1> ret{{
                {d2d::input::combination{{}, d2d::input::mouse_code::button_1}, d2d::input::event_set{{0, event::mouse_release}, {0b1 << category::ui}}},
            }};
            return ret;
        }
    };
}