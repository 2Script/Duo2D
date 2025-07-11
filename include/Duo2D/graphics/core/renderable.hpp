#pragma once
#include "Duo2D/traits/renderable_textures.hpp"
#include "Duo2D/traits/renderable_event_callbacks.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

namespace d2d {
    template<typename T>
    struct renderable : public renderable_traits<T>, public impl::renderable_textures<T>, public impl::renderable_event_callbacks {};
}