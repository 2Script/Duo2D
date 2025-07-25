#pragma once
#include "Duo2D/vulkan/memory/renderable_data_tuple_wrapper.hpp"


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    constexpr bool const& renderable_data_tuple_wrapper<FiF, Ts...>::has_changes() const noexcept {
        return renderable_data_of<T>().has_changes();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    constexpr bool& renderable_data_tuple_wrapper<FiF, Ts...>::has_changes() noexcept {
        return renderable_data_of<T>().has_changes();
    }


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::end() noexcept {
        return renderable_data_of<T>().end();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::const_iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::end() const noexcept {
        return renderable_data_of<T>().end();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::const_iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::cend() const noexcept {
        return renderable_data_of<T>().cend();
    }
    

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::begin() noexcept {
        return renderable_data_of<T>().begin();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::const_iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::begin() const noexcept {
        return renderable_data_of<T>().begin();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    renderable_data_tuple_wrapper<FiF, Ts...>::const_iterator<T> renderable_data_tuple_wrapper<FiF, Ts...>::cbegin() const noexcept {
        return renderable_data_of<T>().cbegin();
    }
}

namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    bool renderable_data_tuple_wrapper<FiF, Ts...>::empty() const noexcept {
        return renderable_data_of<T>().empty();
    }
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    std::size_t renderable_data_tuple_wrapper<FiF, Ts...>::size() const noexcept {
        return renderable_data_of<T>().size();
    }
}


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T> 
    constexpr const pipeline<T>& renderable_data_tuple_wrapper<FiF, Ts...>::associated_pipeline() const noexcept{ 
        return renderable_data_of<T>().pl;
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T> 
    constexpr const pipeline_layout<T>& renderable_data_tuple_wrapper<FiF, Ts...>::associated_pipeline_layout() const noexcept{ 
        return renderable_data_of<T>().pl_layout;
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T> 
    constexpr const std::array<VkDescriptorSet, FiF>& renderable_data_tuple_wrapper<FiF, Ts...>::desc_set() const noexcept{ 
        return renderable_data_of<T>().sets;
    }
}



namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    constexpr renderable_data<T, FiF>      & renderable_data_tuple_wrapper<FiF, Ts...>::renderable_data_of()       noexcept { 
        return std::get<renderable_data<T, FiF>>(renderable_datas);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts>
    template<typename T>
    constexpr renderable_data<T, FiF> const& renderable_data_tuple_wrapper<FiF, Ts...>::renderable_data_of() const noexcept { 
        return std::get<renderable_data<T, FiF>>(renderable_datas); 
    }
}