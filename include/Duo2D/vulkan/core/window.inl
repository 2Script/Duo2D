#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/vulkan/core/window.hpp"


namespace d2d {
    template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
    bool window::insert(const window::value_type<R>& value) noexcept {
        using T = std::remove_cvref_t<R>;
        if(!renderable_mapping.emplace(value.first, data.size<T>()).second)
            return false;
        data.push_back(value.second);
        return true;
    }
    
    template<typename P> requires std::is_constructible_v<window::value_type<typename std::remove_cvref_t<P>::second_type>, P&&>
    bool window::insert(P&& value) noexcept {
        using T = typename std::remove_cvref_t<P>::second_type;
        if(!renderable_mapping.emplace(std::forward<P>(value).first, data.size<T>()).second)
            return false;
        data.emplace_back<T>(std::forward<P>(value).second);
        return true;
    }
    
    template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
    bool window::insert(window::value_type<R>&& value) noexcept {
        using T = std::remove_cvref_t<R>;
        if(!renderable_mapping.emplace(std::move(value.first), data.size<T>()).second)
            return false;
        data.push_back(std::move(value.second));
        return true;
    }

    template<typename T, typename S, typename... Args> requires std::is_constructible_v<std::string, S&&>
    bool window::emplace(S&& str, Args&&... args) noexcept {
        if(!renderable_mapping.emplace(std::forward<S>(str), data.size<T>()).second)
            return false;
        data.emplace_back<T>(std::forward<Args>(args)...);
        return true;
    }
}

namespace d2d {
    template<typename T>
    std::size_t window::erase(std::string_view key) noexcept {
        auto search = renderable_mapping.find(std::string(key));
        if (search == renderable_mapping.end()) return 0;

        std::size_t idx = search->second;
        renderable_mapping.erase(search);
        data.erase<T>(data.cbegin<T>() + idx);
        
        return 1;
    }
}

namespace d2d {
    template<typename T>
    result<void> window::apply(bool shrink) noexcept {
        return data.apply<T>(shrink);
    }
}