#pragma once
#include <type_traits>

#include "Duo2D/input/interactable.hpp"


namespace d2d::impl {
    template<typename T, typename R = std::remove_cvref_t<T>>
    concept interactable_like = requires (T t, d2d::pt2d p) {
        requires std::is_base_of_v<interactable, R>;
        {t.contains(p)} noexcept -> std::same_as<bool>;
    };
}