#pragma once
#include <array>
#include <functional>
#include <iterator>
#include <memory>
#include <result.hpp>
#include <type_traits>
#include <utility>
#include <vector>
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/traits/different_from.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_event_callbacks.hpp"


namespace d2d::impl {
    //TODO: allow multiple children types, and also allow renderable containers as children
    template<impl::directly_renderable ChildT, typename CollectionT>
    class renderable_container_base : public CollectionT, public impl::renderable_event_callbacks {
    public:
        using value_type = typename CollectionT::value_type;
        using element_type = ChildT;
    public:
        using CollectionT::CollectionT;

    public:
        template<typename U = ChildT, typename... Ts>
        constexpr void on_window_insert_child(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<U> inserted_renderable_iter, std::size_t container_idx) noexcept {
            (*this)[container_idx] = hybrid_ptr<ChildT>(std::ref(inserted_renderable_iter->second));
        }
    };
}



namespace d2d {
    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT = std::array>
    class renderable_container : public impl::renderable_container_base<ChildT, ContainerT<hybrid_ptr<ChildT>, N>> {
    public:
        using impl::renderable_container_base<ChildT, ContainerT<hybrid_ptr<ChildT>, N>>::renderable_container_base;
    };
}


namespace d2d {
    template<typename T, impl::directly_renderable ChildT, template<typename...> typename ContainerT = std::vector>
    class dynamic_renderable_container : private impl::renderable_container_base<ChildT, ContainerT<hybrid_ptr<ChildT>>> {
        template<typename V>
        using input_map_iterator_type = typename vk::impl::renderable_input_map<V>::iterator;
        using iterator_vector_type = std::vector<input_map_iterator_type<ChildT>>;
        using input_data_type = vk::impl::renderable_input_data<ChildT>;
        using base_type = impl::renderable_container_base<ChildT, ContainerT<hybrid_ptr<ChildT>>>;
    public:
        using allocator_type = typename base_type::allocator_type;
        using value_type     = typename base_type::value_type;
        using element_type   = typename base_type::element_type;


    public:
        constexpr dynamic_renderable_container() noexcept = default;
        constexpr dynamic_renderable_container(dynamic_renderable_container&& other) noexcept :
            base_type(std::move(other)), child_iters(std::move(other.child_iters)), child_input_data_ptr(std::exchange(other.child_input_data_ptr, nullptr)),
            window_ptr(std::exchange(other.window_ptr, nullptr)), self_iter(std::move(other.self_iter)), apply_changes_fn(std::exchange(other.apply_changes_fn, nullptr)), insert_children_fn(std::exchange(other.insert_children_fn, nullptr)), live(std::exchange(other.live, false)) {}
        constexpr dynamic_renderable_container& operator=(dynamic_renderable_container&& other) noexcept {
            base_type::operator=(std::move(other)); child_iters = std::move(other.child_iters); child_input_data_ptr = std::exchange(other.child_input_data_ptr, nullptr);
            window_ptr = std::exchange(other.window_ptr, nullptr); self_iter = std::move(other.self_iter); apply_changes_fn = std::exchange(other.apply_changes_fn, nullptr); insert_children_fn = std::exchange(other.insert_children_fn, nullptr); live = std::exchange(other.live, false);
        }

    public:
        constexpr dynamic_renderable_container(dynamic_renderable_container const& other) noexcept : 
            base_type(other), child_iters(other.child_iters), child_input_data_ptr(other.child_input_data_ptr), 
            window_ptr{}, self_iter{}, apply_changes_fn{}, insert_children_fn{}, live(false) {}
        constexpr dynamic_renderable_container& operator=(dynamic_renderable_container const& other) noexcept {
            base_type::operator=(other); child_iters = other.child_iters; child_input_data_ptr = other.child_input_data_ptr; 
            window_ptr = {}, self_iter = {}, apply_changes_fn = {}, insert_children_fn = {}; live = false;
        }

    public:
        constexpr explicit dynamic_renderable_container(allocator_type const& alloc) noexcept :
            base_type(alloc), child_iters(), child_input_data_ptr{},
            window_ptr{}, self_iter(), apply_changes_fn{}, insert_children_fn{}, live(false) {}

        template<impl::when_decayed_different_from<std::size_t> Comp>
        constexpr explicit dynamic_renderable_container(Comp const& comp, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<base_type, Comp const&, allocator_type const&> : 
            base_type(comp, alloc), child_iters(), child_input_data_ptr{},
            window_ptr{}, self_iter(), apply_changes_fn{}, insert_children_fn{}, live(false) {}

        constexpr explicit dynamic_renderable_container(std::size_t count, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<base_type, std::size_t, allocator_type const&> :
            base_type(count, alloc), child_iters(), child_input_data_ptr{},
            window_ptr{}, self_iter(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(count); }

        constexpr dynamic_renderable_container(std::size_t count, const hybrid_ptr<ChildT>& value, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<base_type, std::size_t, const hybrid_ptr<ChildT>&, allocator_type const&> :
            base_type(count, value, alloc), child_iters(), child_input_data_ptr{}, 
            window_ptr{}, self_iter(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(count); }

        template<typename InputIt, typename Arg1, typename... Args>
        constexpr dynamic_renderable_container(InputIt first, InputIt last, Arg1 const& arg1, Args const&... args) noexcept requires std::is_constructible_v<base_type, InputIt, InputIt, Arg1 const&, Args const&...> :
            base_type(first, last, arg1, args...), child_iters(), child_input_data_ptr{}, 
            window_ptr{}, self_iter(), apply_changes_fn{}, insert_children_fn{}, live(false) { child_iters.reserve(std::distance(first, last)); }
        

    public:
        ~dynamic_renderable_container() noexcept {
            if(!child_input_data_ptr) return;
            for(auto iter : child_iters)
                child_input_data_ptr->erase(iter);
        }


    public:
        //TODO
        using base_type::at;
        using base_type::operator[];
        //using base_type::front;
        //using base_type::back;
        //using base_type::data;

        //using base_type::begin;
        //using base_type::cbegin;
        //using base_type::end;
        //using base_type::cend;
        //using base_type::rbegin;
        //using base_type::crbegin;
        //using base_type::rend;
        //using base_type::crend;

        using base_type::empty;
        using base_type::size;
        using base_type::max_size;
        using base_type::capacity;

    
    public:
        template<typename... ChildArgs>
        result<void> resize(std::size_t count, ChildArgs&&... child_args) noexcept {
            if constexpr(sizeof...(ChildArgs) == 0)
                base_type::resize(count, make_hybrid_for_overwrite<ChildT>());
            else
                base_type::resize(count, make_hybrid<ChildT>(std::forward<ChildArgs>(child_args)...));
        
            if(!inserted()) return {};
            erase_children_from_last_window();
            insert_children_into_last_window();

            if(!active()) return {};
            RESULT_VERIFY(apply_changes_to_last_window());
            return {};
        }

    protected:
        template<typename U = T, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>& win, typename basic_window<Ts...>::template iterator<U> inserted_iter) noexcept {
            window_ptr = static_cast<void*>(&win);
            self_iter = std::make_unique<input_map_iterator_type<U>>(inserted_iter);
            insert_children_fn = [](void* win_ptr, input_map_iterator_type<T> iter){ return static_cast<basic_window<Ts...>*>(win_ptr)->template insert_children<T>(iter); };
            apply_changes_fn = [](void* win_ptr){ return static_cast<basic_window<Ts...>*>(win_ptr)->template apply_changes<T>(); };
        }

        template<typename U = ChildT, typename... Ts>
        constexpr void on_window_insert_child(basic_window<Ts...>& win, typename basic_window<Ts...>::template iterator<U> inserted_child_iter, std::size_t container_idx) noexcept {
            base_type::on_window_insert_child(win, inserted_child_iter, container_idx);
            child_iters.push_back(inserted_child_iter);
            child_input_data_ptr = &(win.template renderable_data_of<ChildT>());
        }

        template<typename... Ts>
        constexpr result<void> after_changes_applied(basic_window<Ts...> const&) noexcept {
            live = true;
            return {}; 
        }

        //TODO?: find a better way to access renderable_callback_event functions (maybe make them all publicly inherited?)
        template<typename... Ts>
        friend struct basic_window;


    protected:
        constexpr bool inserted() const noexcept { return window_ptr; }
        constexpr bool active() const noexcept { return live; }

    protected:
        result<void> apply_changes_to_last_window() const noexcept {
            if(!apply_changes_fn) return {};
            return apply_changes_fn(window_ptr);
        }

        constexpr bool insert_children_into_last_window() const noexcept {
            if(!insert_children_fn) return false;
            return insert_children_fn(window_ptr, *self_iter);
        }

        inline void erase_children_from_last_window() noexcept {
            if(!child_input_data_ptr) return;
            for(auto iter : child_iters)
                child_input_data_ptr->erase(iter);
            child_iters.clear();
            child_input_data_ptr = nullptr;
        }


    private:
        //TODO user iterators (e.g. an iterator_wrapper class) in the main child container so that this vector isn't needed anymore
        std::vector<input_map_iterator_type<ChildT>> child_iters;
        input_data_type* child_input_data_ptr;

    private:
        void* window_ptr;
        std::unique_ptr<input_map_iterator_type<T>> self_iter;
        result<void>(*apply_changes_fn)(void*);
        bool(*insert_children_fn)(void*, input_map_iterator_type<T>);
        bool live;
    };
}