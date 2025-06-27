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
    template<typename T, typename R = std::remove_cvref_t<T>>
    concept renderable_like = requires {
        typename R::traits_type;
        requires shader_like<typename R::shader_type>;
        requires std::is_standard_layout_v<typename R::instance_type> || !renderable_constraints<R>::instanced;
        requires (requires(R r) {{r.instance()} noexcept -> std::same_as<typename R::instance_type>;}) || !renderable_constraints<R>::instanced;
        requires (requires(R r) {{r.vertices()} noexcept;}) || renderable_constraints<R>::instanced;
    };
    template<typename T>
    concept file_renderable_like = renderable_like<T> && (requires(T t) {{t.unload()} noexcept;});
}

