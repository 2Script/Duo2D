#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/window.hpp"

#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/pipeline/shader_buffer.hpp"


namespace d2d {
    template<impl::RenderableType T>
    result<void> window::add(std::string_view name, T&& renderable)  {
        constexpr static std::array<std::uint32_t, 6> indicies = T::indicies();
        std::array<vertex2, 4> verticies = renderable.verticies(static_cast<size2f>(_swap_chain.extent));

        if(!index_count) {
            __D2D_TRY_MAKE(index_buffer, 
                make<shader_buffer>(*logi_device_ptr, *phys_device_ptr, _command_pool, indicies.data(), indicies.size() * sizeof(std::uint32_t), buffer_type::index), 
            ib);
            index_count = indicies.size();
        }

        renderable_mapping.emplace(name, vertex_buffers.size());
        auto vb = make<shader_buffer>(*logi_device_ptr, *phys_device_ptr, _command_pool, verticies.data(), verticies.size() * sizeof(vertex2), buffer_type::vertex);
        if(!vb.has_value()) return vb.error();
        
        buffer_offsets.push_back(0);
        vk_vertex_buffers.push_back(static_cast<VkBuffer>(*vb));
        vertex_buffers.push_back(*std::move(vb));

        return result<void>{std::in_place_type<void>};
    }
}


namespace d2d {
    result<void> window::remove(std::string_view name) {
        auto search = renderable_mapping.find(std::string(name));
        if (search == renderable_mapping.end())
            return error::element_not_found;
        std::size_t idx = search->second;
        renderable_mapping.erase(search);
        buffer_offsets.erase(buffer_offsets.cbegin() + idx);
        vk_vertex_buffers.erase(vk_vertex_buffers.cbegin() + idx);
        vertex_buffers.erase(vertex_buffers.cbegin() + idx);
        return result<void>{std::in_place_type<void>};
    }
}