#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/core/window.hpp"


namespace d2d {
    template<typename R> requires impl::renderable_like<std::remove_cvref_t<R>>
    std::pair<window::iterator<R>, bool> window::insert(const window::value_type<R>& value) noexcept {
        using T = std::remove_cvref_t<R>;
        auto ins = data.renderable_data_of<T>().input_renderables.insert(value);
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }

    template<typename R> requires impl::renderable_like<std::remove_cvref_t<R>>
    std::pair<window::iterator<R>, bool> window::insert(window::value_type<R>&& value) noexcept {
        using T = std::remove_cvref_t<R>;
        auto ins = data.renderable_data_of<T>().input_renderables.insert(std::move(value));
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }
    
    template<typename P> requires std::is_constructible_v<window::value_type<typename std::remove_cvref_t<P>::second_type>, P&&>
    std::pair<window::iterator<typename std::remove_cvref_t<P>::second_type>, bool> window::insert(P&& value) noexcept {
        using T = typename std::remove_cvref_t<P>::second_type;
        auto ins = data.renderable_data_of<T>().input_renderables.insert(std::forward<P>(value));
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }


    template<typename T, typename... Args>
    std::pair<window::iterator<T>, bool> window::emplace(Args&&... args) noexcept {
        auto ins = data.renderable_data_of<T>().input_renderables.emplace(std::forward<Args>(args)...);
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }

    template<typename T, typename... Args>
    std::pair<window::iterator<T>, bool> window::try_emplace(std::string_view str, Args&&... args) noexcept {
        auto ins = data.renderable_data_of<T>().input_renderables.try_emplace(str, std::forward<Args>(args)...);
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }

    template<typename T, typename S, typename... Args> requires std::is_constructible_v<std::string, S&&>
    std::pair<window::iterator<T>, bool> window::try_emplace(S&& str, Args&&... args) noexcept {
        auto ins = data.renderable_data_of<T>().input_renderables.try_emplace(std::forward<S>(str), std::forward<Args>(args)...);
        if(ins.second) data.renderable_data_of<T>().outdated = true;
        return ins;
    }
}

namespace d2d {
    template<typename T>
    window::iterator<T> window::erase(window::iterator<T> pos) noexcept {
        data.renderable_data_of<T>().outdated = true;
        return data.renderable_data_of<T>().input_renderables.erase(pos);
    }
    template<typename T>
    window::iterator<T> window::erase(window::const_iterator<T> pos) noexcept {
        data.renderable_data_of<T>().outdated = true;
        return data.renderable_data_of<T>().input_renderables.erase(pos);
    }
    template<typename T>
    window::iterator<T> window::erase(window::const_iterator<T> first, window::const_iterator<T> last) noexcept {
        data.renderable_data_of<T>().outdated = true;
        return data.renderable_data_of<T>().input_renderables.erase(first, last);
    }
    template<typename T>
    std::size_t window::erase(std::string_view key) noexcept {
        std::size_t erased_count = data.renderable_data_of<T>().input_renderables.erase(key);
        if(erased_count) data.renderable_data_of<T>().outdated = true;
        return erased_count;
    }
}

namespace d2d {
    template<typename T>
    window::iterator<T> window::end() noexcept {
        return data.renderable_data_of<T>().input_renderables.end();
    }
    template<typename T>
    window::const_iterator<T> window::end() const noexcept {
        return data.renderable_data_of<T>().input_renderables.end();
    }
    template<typename T>
    window::const_iterator<T> window::cend() const noexcept {
        return data.renderable_data_of<T>().input_renderables.cend();
    }
}

namespace d2d {
    template<typename T>
    window::iterator<T> window::begin() noexcept {
        return data.renderable_data_of<T>().input_renderables.begin();
    }
    template<typename T>
    window::const_iterator<T> window::begin() const noexcept {
        return data.renderable_data_of<T>().input_renderables.begin();
    }
    template<typename T>
    window::const_iterator<T> window::cbegin() const noexcept {
        return data.renderable_data_of<T>().input_renderables.cbegin();
    }
}

namespace d2d {
    template<typename T>
    bool window::empty() const noexcept {
        return data.renderable_data_of<T>().input_renderables.empty();
    }
    template<typename T>
    std::size_t window::size() const noexcept {
        return data.renderable_data_of<T>().input_renderables.size();
    }
}

namespace d2d {
    template<typename T>
    result<void> window::apply() noexcept {
        return data.apply<T>();
    }
}