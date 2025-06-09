#pragma once
#include "Duo2D/traits/renderable_constraints.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include <concepts>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<typename T>
    struct renderable_traits;
}


namespace d2d::impl {
    template<typename T>
    concept renderable_like = requires {
        typename T::traits_type;
        requires shader_like<typename T::shader_type>;
        requires std::is_standard_layout_v<typename T::instance_type> || !renderable_constraints<T>::instanced;
        requires (requires(T t) {{t.instance()} noexcept -> std::same_as<typename T::instance_type>;}) || !renderable_constraints<T>::instanced;
        requires (requires(T t) {{t.vertices()} noexcept;}) || renderable_constraints<T>::instanced;
    };
    template<typename T>
    concept file_renderable_like = renderable_like<T> && (requires(T t) {{t.unload()} noexcept;});
}

