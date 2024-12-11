#pragma once
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/renderable_buffer.hpp"

namespace d2d {
    template<impl::RenderableType T, std::size_t FiF, impl::RenderableType... Rs>
    result<void> command_buffer::draw(const renderable_buffer<FiF, Rs...>& renderables) const noexcept {
        if(std::get<std::vector<T>>(renderables).empty())
            return result<void>{std::in_place_type<void>};
        
        //Set pipeline
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline<T>());

        //Bind instance buffer
        if constexpr (T::instanced) {
            const VkBuffer vk_instance_buffer = static_cast<VkBuffer>(renderables.template instance_buffer<T>());
            constexpr static std::size_t instance_offset = renderable_buffer<FiF, Rs...>::template offsets<T>().instance;
            if constexpr (impl::VertexRenderableType<T>)
                vkCmdBindVertexBuffers(handle, 1, 1, &vk_instance_buffer, &instance_offset);
            else
                vkCmdBindVertexBuffers(handle, 0, 1, &vk_instance_buffer, &instance_offset);
        } 

        //Bind vertex buffer
        if constexpr (!T::instanced || impl::VertexRenderableType<T>) {
            const VkBuffer vk_vertex_buffer = static_cast<VkBuffer>(renderables.template vertex_buffer<T>());
            constexpr static std::size_t vertex_offset = renderable_buffer<FiF, Rs...>::template offsets<T>().vertex;
            vkCmdBindVertexBuffers(handle, 0, 1, &vk_vertex_buffer, &vertex_offset);
        }

        //Bind index buffer
        if constexpr (impl::IndexRenderableType<T>) 
            vkCmdBindIndexBuffer(handle, static_cast<VkBuffer>(renderables.template index_buffer<T>()), renderable_buffer<FiF, Rs...>::template offsets<T>().index, sizeof(typename T::index_type) == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        
        //Bind descriptor set
        if constexpr (impl::UniformRenderableType<T>)
            vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline_layout<T>(), 0, 1, renderables.template desc_set<T>().data(), 0, nullptr);

        //Draw vertices
        if constexpr (impl::IndexRenderableType<T>) 
            vkCmdDrawIndexed(handle, renderables.template index_count<T>(), renderables.template instance_count<T>(), 0, 0, 0);
        else
            vkCmdDraw(handle, renderables.template vertex_count<T>(), renderables.template instance_count<T>(), 0, 0);

        return result<void>{std::in_place_type<void>};
    }
}