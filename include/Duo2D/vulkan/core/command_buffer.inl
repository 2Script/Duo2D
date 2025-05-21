#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"
#include <memory>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<impl::RenderableType T, std::size_t FiF, impl::RenderableType... Rs>
    result<void> command_buffer::draw(const renderable_buffer<FiF, Rs...>& renderables) const noexcept {
        if(renderables.template empty<T>())
            return {};

        if(renderables.template needs_apply<T>())
            return error::buffer_needs_changes_applied;
        
        //Set pipeline
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline<T>());

        //Bind instance buffer
        if constexpr (T::instanced) {
            const VkBuffer vk_instance_buffer = static_cast<VkBuffer>(renderables.template instance_buffer<T>());
            constexpr static std::size_t instance_offset = renderable_buffer<FiF, Rs...>::template static_offsets<T>()[buffer_data_type::instance];
            vkCmdBindVertexBuffers(handle, impl::has_vertices_v<T>, 1, &vk_instance_buffer, &instance_offset);
        } 

        //Bind vertex buffer
        if constexpr (!T::instanced || impl::has_vertices_v<T>) {
            const VkBuffer vk_vertex_buffer = static_cast<VkBuffer>(renderables.template vertex_buffer<T>());
            const std::size_t vertex_offset = T::instanced ? 
                renderable_buffer<FiF, Rs...>::template static_offsets<T>()[buffer_data_type::vertex] : 
                renderables.template vertex_buffer_offset<T>();
            vkCmdBindVertexBuffers(handle, 0, 1, &vk_vertex_buffer, &vertex_offset);
        }

        //Bind attribute buffer
        if constexpr (impl::has_attributes_v<T>) {
            const VkBuffer vk_attrib_buffer = static_cast<VkBuffer>(renderables.template attribute_buffer<T>());
            constexpr static std::size_t attribute_binding = impl::has_vertices_v<T> + T::instanced;
            constexpr static std::size_t attribute_offset = 0;//renderable_buffer<FiF, Rs...>::template static_offsets<T>()[buffer_data_type::attribute];
            vkCmdBindVertexBuffers(handle, attribute_binding, 1, &vk_attrib_buffer, &attribute_offset);
        }

        //Bind index buffer
        if constexpr (impl::has_indices_v<T>) {
            constexpr static std::size_t index_offset = T::instanced ? renderable_buffer<FiF, Rs...>::template static_offsets<T>()[buffer_data_type::index] : 0;
            vkCmdBindIndexBuffer(handle, static_cast<VkBuffer>(renderables.template index_buffer<T>()), index_offset, sizeof(typename T::index_type) == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        }

        //Bind descriptor set
        if constexpr (impl::has_uniform_v<T>)
            vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline_layout<T>(), 0, 1, renderables.template desc_set<T>().data(), 0, nullptr);

        //Set push constants
        if constexpr (impl::has_push_constants_v<T>) {
            [this, &renderables]<std::size_t... Is>(std::index_sequence<Is...>) {
                constexpr static std::array pcr = T::push_constant_ranges();
                ([this, &renderables]<std::size_t I>(std::integral_constant<std::size_t, I>){
                    static auto push_consts = std::get<I>(T::push_constants());
                    vkCmdPushConstants(handle, renderables.template associated_pipeline_layout<T>(), pcr[I].stageFlags, pcr[I].offset, pcr[I].size, std::addressof(push_consts));
                }(std::integral_constant<std::size_t, Is>{}), ...);
            }(std::make_index_sequence<T::push_constant_ranges().size()>{});
        }

        //Draw vertices
        if constexpr (T::instanced) {
            if constexpr (impl::has_indices_v<T>) 
                vkCmdDrawIndexed(handle, T::index_count, renderables.template instance_count<T>(), 0, 0, 0);
            else
                vkCmdDraw(handle, T::vertex_count, renderables.template instance_count<T>(), 0, 0);
        }
        else {
            for(std::size_t i = 0; i < renderables.template instance_count<T>(); ++i) {
                if constexpr (impl::has_indices_v<T>) 
                    vkCmdDrawIndexed(handle, renderables.template index_count<T>(i), 1, renderables.template first_index<T>(i), renderables.template first_vertex<T>(i), i);
                else
                    vkCmdDraw(handle, renderables.template vertex_count<T>(i), 1, renderables.template first_vertex<T>(i), i);
            }
        }

        return {};
    }
}