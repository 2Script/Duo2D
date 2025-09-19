#pragma once
#include <array>
#include <concepts>
#include <ranges>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include <result.hpp>

#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/traits/different_from.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include "Duo2D/traits/renderable_event_callbacks.hpp"


namespace d2d {
    template<typename ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT = std::array>
    class renderable_container {};

    template<typename ChildT, template<typename...> typename ContainerT = std::vector>
    class dynamic_renderable_container {};
}



namespace d2d {
    template<impl::directly_renderable ChildT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
    struct renderable_container<ChildT, N, ContainerT> : public ContainerT<hybrid_ptr<ChildT>, N>, public impl::renderable_event_callbacks {
        using container_type = ContainerT<hybrid_ptr<ChildT>, N>;
        using renderable_type = ChildT;
        using input_map_key_type = typename vk::impl::renderable_input_map<ChildT>::key_type;
        using value_type             = typename container_type::value_type;
        using iterator               = typename container_type::iterator;
        using const_iterator         = typename container_type::const_iterator;
        using reverse_iterator       = typename container_type::reverse_iterator;
        using const_reverse_iterator = typename container_type::const_reverse_iterator;

    public:
        constexpr renderable_container() noexcept = default;

        template<typename... ChildHybridPtrs>
        constexpr renderable_container(ChildHybridPtrs&&... child_hybrid_ptrs) noexcept requires (sizeof...(ChildHybridPtrs) == N && (std::is_constructible_v<hybrid_ptr<ChildT>, ChildHybridPtrs&&> && ...));

        constexpr renderable_container(container_type const& container) noexcept;
        constexpr renderable_container(container_type     && container) noexcept;
    public:
        constexpr renderable_container(renderable_container&& other) noexcept = default;
        constexpr renderable_container& operator=(renderable_container&& other) noexcept = default;
        constexpr renderable_container(renderable_container const& other) noexcept = default;
        constexpr renderable_container& operator=(renderable_container const& other) noexcept = default;

    public:
        ~renderable_container() noexcept;

    public:
        template<typename U, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>& win, input_map_key_type insertion_key) noexcept;

        template<typename U = ChildT, typename... Ts>
        constexpr void on_window_insert_child_renderable(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<U> inserted_renderable_iter, std::size_t container_idx) noexcept;
    
    private:
        //TODO user iterators (e.g. an iterator_wrapper class) in the main child container so that this vector isn't needed anymore
        input_map_key_type self_key = 0;
        ContainerT<input_map_key_type, N> child_keys{};
        vk::impl::renderable_input_data<ChildT>* child_input_data_ptr = nullptr;
    private:
        template<typename... Ts>
        friend struct basic_window;
    };

    //TODO?
    //template<impl::renderable_container_tuple_like RenderableContainerT, std::size_t N, template<typename, std::size_t...> typename ContainerT>
    //class renderable_container<RenderableContainerT, N, ContainerT> : public RenderableContainerT {};
}


namespace d2d {
    template<impl::directly_renderable ChildT, template<typename...> typename ContainerT>
    class dynamic_renderable_container<ChildT, ContainerT> : public ContainerT<hybrid_ptr<ChildT>>, public impl::renderable_event_callbacks {
        //TODO: implement an alternative to this
        //static_assert(std::is_base_of_v<dynamic_renderable_container<T, ChildT, ContainerT>, T>, "dynamic_renderable_container<T, ...> must be a base class of T");
    public:
        using input_map_key_type = typename vk::impl::renderable_input_map<ChildT>::key_type;
        using key_vector_type = std::vector<input_map_key_type>;
        using input_data_type = vk::impl::renderable_input_data<ChildT>;
    public:
        using container_type = ContainerT<hybrid_ptr<ChildT>>;
        using renderable_type = ChildT;
        using allocator_type  = typename container_type::allocator_type;
        using value_type      = typename container_type::value_type;


    public:
        constexpr dynamic_renderable_container() noexcept = default;
        constexpr dynamic_renderable_container(dynamic_renderable_container&& other) noexcept;
        constexpr dynamic_renderable_container& operator=(dynamic_renderable_container&& other) noexcept;

    public:
        constexpr dynamic_renderable_container(dynamic_renderable_container const& other) noexcept;
        constexpr dynamic_renderable_container& operator=(dynamic_renderable_container const& other) noexcept;

    public:
        constexpr explicit dynamic_renderable_container(allocator_type const& alloc) noexcept;

        template<impl::when_decayed_different_from<std::size_t> Comp>
        constexpr explicit dynamic_renderable_container(Comp const& comp, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<container_type, Comp const&, allocator_type const&>;
        constexpr explicit dynamic_renderable_container(std::size_t count, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<container_type, std::size_t, allocator_type const&>;

        constexpr dynamic_renderable_container(std::size_t count, const hybrid_ptr<ChildT>& value, allocator_type const& alloc = allocator_type()) noexcept requires std::is_constructible_v<container_type, std::size_t, const hybrid_ptr<ChildT>&, allocator_type const&>;

        template<typename InputIt, typename Arg1, typename... Args>
        constexpr dynamic_renderable_container(InputIt first, InputIt last, Arg1 const& arg1, Args const&... args) noexcept requires std::is_constructible_v<container_type, InputIt, InputIt, Arg1 const&, Args const&...>;
        

    public:
        ~dynamic_renderable_container() noexcept;

    
    public:
        template<typename... ChildArgs>
        result<void> resize(std::size_t count, ChildArgs&&... child_args) noexcept;
        //TODO
        //inline void clear() noexcept;

    protected:
        template<typename U, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>& win, input_map_key_type insertion_key) noexcept;

        template<typename U = ChildT, typename... Ts>
        constexpr void on_window_insert_child_renderable(basic_window<Ts...>& win, typename basic_window<Ts...>::template iterator<U> inserted_child_iter, std::size_t container_idx) noexcept;

        template<typename... Ts>
        constexpr result<void> after_changes_applied(basic_window<Ts...> const&) noexcept;

        //TODO?: find a better way to access renderable_callback_event functions (maybe make them all publicly inherited?)
        template<typename... Ts>
        friend struct basic_window;

        template<typename... RenderableContainerTs>
        friend struct renderable_container_tuple;


    protected:
        constexpr bool inserted() const noexcept { return window_ptr; }
        constexpr bool active() const noexcept { return live; }

    protected:
        result<void> apply_changes_to_last_window() const noexcept;
        constexpr bool insert_children_into_last_window() noexcept;
        inline void erase_children_from_last_window() noexcept;


    private:
        //TODO user iterators (e.g. an iterator_wrapper class) in the main child container so that this vector isn't needed anymore
        key_vector_type child_keys;
        input_data_type* child_input_data_ptr;

    private:
        void* window_ptr;
        input_map_key_type self_key;
        result<void>(*apply_changes_fn)(void*);
        bool(*insert_children_fn)(void*, input_map_key_type, void*);
        bool live;
    };
}


namespace d2d {
    //TODO flatten tuples in RenderableContainerTs to allow nested `renderable_container_tuple`s
    template<typename... RenderableContainerTs>
    struct renderable_container_tuple : public std::tuple<hybrid_ptr<RenderableContainerTs>...>, public impl::renderable_event_callbacks {
        using tuple_type = std::tuple<RenderableContainerTs...>;
        using base_type = std::tuple<hybrid_ptr<RenderableContainerTs>...>;
    public:
        //NOTE: no destructor to remove child container, since this whole system will probably be reworked later (i.e. tuples and containers will be merged)

        using base_type::base_type;

        constexpr static std::size_t container_count = sizeof...(RenderableContainerTs);

        template<std::size_t I>
        using container_type = std::tuple_element_t<I, tuple_type>;
        template<std::size_t I>
        using value_type = std::tuple_element_t<I, base_type>;

        template<std::size_t I>
        constexpr value_type<I>& get_container() noexcept;
        template<std::size_t I>
        constexpr value_type<I> const& get_container() const noexcept;


    public:
        template<typename U, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>& win, std::uint64_t insertion_key) noexcept;

        template<std::size_t I, typename... Ts>
        constexpr void on_window_insert_child_container(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<container_type<I>> inserted_child_iter) noexcept;
    
    private:
        std::uint64_t self_key = 0;
    private:
        template<typename... Ts>
        friend struct basic_window;
    };
}

#include "Duo2D/graphics/core/renderable_container.inl"
