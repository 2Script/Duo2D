#pragma once
#include <cstddef>
#include <tuple>
#include <unordered_map>

#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"


namespace d2d::vk {
    template<std::size_t FramesInFlight, ::d2d::impl::directly_renderable... Ts>
    class renderable_data_tuple_wrapper {
    public:
        template<typename T> using renderable_data_type = renderable_data<T, FramesInFlight>;
        using renderable_data_tuple_type = std::tuple<renderable_data_type<Ts>...>;


    public:
        template<typename T> using key_type       = typename renderable_data_type<T>::key_type;
        template<typename T> using mapped_type    = typename renderable_data_type<T>::mapped_type;
        template<typename T> using value_type     = typename renderable_data_type<T>::value_type;
        template<typename T> using iterator       = typename renderable_data_type<T>::iterator;
        template<typename T> using const_iterator = typename renderable_data_type<T>::const_iterator;

        template<typename T>
        constexpr bool const& has_changes() const noexcept;
        template<typename T>
        constexpr bool      & has_changes()       noexcept;

        template<typename T>       iterator<T> end()        noexcept;
        template<typename T> const_iterator<T> end()  const noexcept;
        template<typename T> const_iterator<T> cend() const noexcept;

        template<typename T>       iterator<T> begin()        noexcept;
        template<typename T> const_iterator<T> begin()  const noexcept;
        template<typename T> const_iterator<T> cbegin() const noexcept;

        template<typename T> bool empty() const noexcept;
        template<typename T> std::size_t size() const noexcept;

    protected:
        template<typename T> constexpr const pipeline<T>&                                 associated_pipeline()        const noexcept;
        template<typename T> constexpr const pipeline_layout<T>&                          associated_pipeline_layout() const noexcept;
        template<typename T> constexpr const std::array<VkDescriptorSet, FramesInFlight>& desc_set()                   const noexcept;


    protected:
        template<typename T> constexpr renderable_data<T, FramesInFlight>      & renderable_data_of()       noexcept;
        template<typename T> constexpr renderable_data<T, FramesInFlight> const& renderable_data_of() const noexcept;

        friend struct renderable_event_callbacks;


    protected:
        renderable_data_tuple_type renderable_datas;
    };
}

#include "Duo2D/vulkan/memory/renderable_data_tuple_wrapper.inl"