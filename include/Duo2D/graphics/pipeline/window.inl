#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/window.hpp"

#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/pipeline/shader_buffer.hpp"


namespace d2d {
    template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
    result<void> window::add(std::string_view name, R&& renderable)  {
        using T = std::remove_cvref_t<R>;
        renderable_mapping.emplace(name, std::get<std::vector<T>>(data).size());
        std::get<std::vector<T>>(data).push_back(std::forward<R>(renderable));

        if(auto a = data.apply<T>(); !a.has_value())
            return a.error();

        return result<void>{std::in_place_type<void>};
    }
}


namespace d2d {
    template<impl::RenderableType T>
    result<void> window::remove(std::string_view name) {
        auto search = renderable_mapping.find(std::string(name));
        if (search == renderable_mapping.end())
            return error::element_not_found;

        std::size_t idx = search->second;
        renderable_mapping.erase(search);
        std::get<std::vector<T>>(data).erase(std::get<std::vector<T>>(data).cbegin() + idx);

        if(auto a = data.apply<T>(); !a.has_value())
            return a.error();
        
        return result<void>{std::in_place_type<void>};
    }
}