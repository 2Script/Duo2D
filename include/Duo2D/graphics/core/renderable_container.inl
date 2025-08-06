#pragma once
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/graphics/core/renderable_container.hpp"
#include "Duo2D/traits/renderable_event_callbacks.hpp"
#include <utility>


namespace d2d {
    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
        template<typename... ChildHybridPtrs>
    constexpr renderable_container<ChildT, N, ContainerT>::renderable_container(ChildHybridPtrs&&... child_hybrid_ptrs) noexcept 
    requires (sizeof...(ChildHybridPtrs) == N && (std::is_constructible_v<hybrid_ptr<ChildT>, ChildHybridPtrs&&> && ...)) : 
        container_type{std::forward<ChildHybridPtrs>(child_hybrid_ptrs)...} {}


    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
    constexpr renderable_container<ChildT, N, ContainerT>::renderable_container(container_type const& container) noexcept :
        container_type{container} {}


    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
    constexpr renderable_container<ChildT, N, ContainerT>::renderable_container(container_type     && container) noexcept :
        container_type{std::move(container)} {}
}


namespace d2d {
    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
    template<typename U, typename... Ts>
    constexpr void renderable_container<ChildT, N, ContainerT>::on_window_insert_child_renderable(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<U> inserted_renderable_iter, std::size_t container_idx) noexcept {
        (*this)[container_idx] = hybrid_ptr<ChildT>(std::ref(inserted_renderable_iter->second));
    }
}



namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(dynamic_renderable_container&& other) noexcept :
        container_type(std::move(other)), child_iters(std::move(other.child_iters)), child_input_data_ptr(std::exchange(other.child_input_data_ptr, nullptr)),
        window_ptr(std::exchange(other.window_ptr, nullptr)), self_key(std::move(other.self_key)), apply_changes_fn(std::exchange(other.apply_changes_fn, nullptr)), insert_children_fn(std::exchange(other.insert_children_fn, nullptr)), live(std::exchange(other.live, false)) {}
    
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>& dynamic_renderable_container<T, ChildT, ContainerT>::operator=(dynamic_renderable_container&& other) noexcept {
        container_type::operator=(std::move(other)); child_iters = std::move(other.child_iters); child_input_data_ptr = std::exchange(other.child_input_data_ptr, nullptr);
        window_ptr = std::exchange(other.window_ptr, nullptr); self_key = std::move(other.self_key); apply_changes_fn = std::exchange(other.apply_changes_fn, nullptr); insert_children_fn = std::exchange(other.insert_children_fn, nullptr); live = std::exchange(other.live, false);
        return *this;
    }


    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(dynamic_renderable_container const& other) noexcept : 
        container_type(other), child_iters(other.child_iters), child_input_data_ptr(other.child_input_data_ptr), 
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) {}
        
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>& dynamic_renderable_container<T, ChildT, ContainerT>::operator=(dynamic_renderable_container const& other) noexcept {
        container_type::operator=(other); child_iters = other.child_iters; child_input_data_ptr = other.child_input_data_ptr; 
        window_ptr = {}, self_key = decltype(self_key)(), apply_changes_fn = {}, insert_children_fn = {}; live = false;
        return *this;
    }
}

namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(allocator_type const& alloc) noexcept :
        container_type(alloc), child_iters(), child_input_data_ptr{},
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) {}

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<impl::when_decayed_different_from<std::size_t> Comp>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(Comp const& comp, allocator_type const& alloc) noexcept requires std::is_constructible_v<container_type, Comp const&, allocator_type const&> : 
        container_type(comp, alloc), child_iters(), child_input_data_ptr{},
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) {}

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(std::size_t count, allocator_type const& alloc) noexcept requires std::is_constructible_v<container_type, std::size_t, allocator_type const&> :
        container_type(count, alloc), child_iters(), child_input_data_ptr{},
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(count); }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(std::size_t count, const hybrid_ptr<ChildT>& value, allocator_type const& alloc) noexcept requires std::is_constructible_v<container_type, std::size_t, const hybrid_ptr<ChildT>&, allocator_type const&> :
        container_type(count, value, alloc), child_iters(), child_input_data_ptr{}, 
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(count); }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<typename InputIt, typename Arg1, typename... Args>
    constexpr dynamic_renderable_container<T, ChildT, ContainerT>::dynamic_renderable_container(InputIt first, InputIt last, Arg1 const& arg1, Args const&... args) noexcept requires std::is_constructible_v<container_type, InputIt, InputIt, Arg1 const&, Args const&...> :
        container_type(first, last, arg1, args...), child_iters(), child_input_data_ptr{}, 
        window_ptr{}, self_key(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(std::distance(first, last)); }
}

namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    dynamic_renderable_container<T, ChildT, ContainerT>::~dynamic_renderable_container() noexcept {
        if(!child_input_data_ptr) return;
        for(auto iter : child_iters)
            child_input_data_ptr->erase(iter);
    }
}


namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<typename... ChildArgs>
    result<void> dynamic_renderable_container<T, ChildT, ContainerT>::resize(std::size_t count, ChildArgs&&... child_args) noexcept {
        if constexpr(sizeof...(ChildArgs) == 0)
            container_type::resize(count, make_hybrid_for_overwrite<ChildT>());
        else
            container_type::resize(count, make_hybrid<ChildT>(std::forward<ChildArgs>(child_args)...));
    
        if(!inserted()) return {};
        erase_children_from_last_window();
        insert_children_into_last_window();

        if(!active()) return {};
        RESULT_VERIFY(apply_changes_to_last_window());
        return {};
    }
}
    

namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<typename U, typename... Ts>
    constexpr void dynamic_renderable_container<T, ChildT, ContainerT>::on_window_insert(basic_window<Ts...>& win, std::string_view insertion_key) noexcept {
        window_ptr = static_cast<void*>(&win);
        self_key = insertion_key;
        insert_children_fn = [](void* win_ptr, std::string_view key, T& parent){ return static_cast<basic_window<Ts...>*>(win_ptr)->template insert_children<T>(key, parent); };
        apply_changes_fn = [](void* win_ptr){ return static_cast<basic_window<Ts...>*>(win_ptr)->template apply_changes<T>(); };
    }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<typename U , typename... Ts>
    constexpr void dynamic_renderable_container<T, ChildT, ContainerT>::on_window_insert_child_renderable(basic_window<Ts...>& win, typename basic_window<Ts...>::template iterator<U> inserted_child_iter, std::size_t container_idx) noexcept {
        (*this)[container_idx] = hybrid_ptr<ChildT>(std::ref(inserted_child_iter->second));
        child_iters.push_back(inserted_child_iter);
        child_input_data_ptr = &(win.template renderable_data_of<ChildT>());
    }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    template<typename... Ts>
    constexpr result<void> dynamic_renderable_container<T, ChildT, ContainerT>::after_changes_applied(basic_window<Ts...> const&) noexcept {
        live = true;
        return {}; 
    }
}


namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    result<void> dynamic_renderable_container<T, ChildT, ContainerT>::apply_changes_to_last_window() const noexcept {
        if(!apply_changes_fn) return {};
        return apply_changes_fn(window_ptr);
    }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    constexpr bool dynamic_renderable_container<T, ChildT, ContainerT>::insert_children_into_last_window() noexcept {
        if(!insert_children_fn) return false;
        return insert_children_fn(window_ptr, self_key, *static_cast<T*>(this));
    }

    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    inline void dynamic_renderable_container<T, ChildT, ContainerT>::erase_children_from_last_window() noexcept {
        if(!child_input_data_ptr) return;
        for(std::size_t i = 0; i < std::min(child_iters.size(), this->size()); ++i) {\
            (*this)[i] = make_hybrid<ChildT>(std::move(child_iters[i]->second));
            child_input_data_ptr->erase(child_iters[i]);
        }
        child_iters.clear();
        child_input_data_ptr = nullptr;
    }
}


namespace d2d {
    template<typename... RenderableContainerTs>
    template<std::size_t I>
    constexpr typename renderable_container_tuple<RenderableContainerTs...>::template value_type<I>& renderable_container_tuple<RenderableContainerTs...>::get_container() noexcept {
        return std::get<I>(*this);
    }

    //template<typename... RenderableContainerTs>
    //template<typename U, typename... Ts>
    //constexpr void renderable_container_tuple<RenderableContainerTs...>::on_window_insert_child(basic_window<Ts...>& win, typename basic_window<Ts...>::template iterator<U> inserted_child_iter, std::size_t container_idx) noexcept {
    //    (std::get<RenderableContainerTs>(*this).template on_window_insert_child<U>(win, inserted_child_iter, container_idx), ...);
    //}

    template<typename... RenderableContainerTs>
    template<std::size_t I, typename... Ts>
    constexpr void renderable_container_tuple<RenderableContainerTs...>::on_window_insert_child_container(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<container_type<I>> inserted_child_iter) noexcept {
        std::get<I>(*this) = hybrid_ptr<container_type<I>>(std::ref(inserted_child_iter->second));
    }

    //template<typename... RenderableContainerTs>
    //template<typename... Ts>
    //constexpr result<void> renderable_container_tuple<RenderableContainerTs...>::after_changes_applied(basic_window<Ts...> const& win) noexcept {
    //    return [&, this]<std::size_t... Is>(std::index_sequence<Is...>) noexcept -> result<void> {
    //        RESULT_VERIFY(ol::to_result((std::bind(&RenderableContainerTs::template after_changes_applied<Ts...>, &std::get<Is>(*this), std::cref(win)) && ...)));
    //        return {};
    //    }(std::make_index_sequence<sizeof...(RenderableContainerTs)>{});
    //}
}