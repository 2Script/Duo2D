#pragma once
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/traits/buffer_traits.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    template<::d2d::impl::directly_renderable T, std::size_t FiF, typename TupleT>
    result<void> command_buffer::draw(const renderable_tuple<FiF, TupleT>& renderables) const noexcept {
        if(renderables.template empty<T>())
            return {};

        if(renderables.template has_changes<T>())
            return error::buffer_needs_changes_applied;
        
        //Set pipeline
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline<T>());

        //Bind vertex buffer
        if constexpr (!renderable_constraints<T>::instanced || renderable_constraints<T>::has_vertices) {
            const VkBuffer vk_vertex_buffer = static_cast<VkBuffer>(renderables.template vertex_buffer<T>());
            const std::size_t vertex_offset = renderables.template vertex_buffer_offset<T>();
            vkCmdBindVertexBuffers(handle, renderable_properties<T>::vertex_binding(), 1, &vk_vertex_buffer, &vertex_offset);
        }

        //Bind instance buffer
        if constexpr (renderable_constraints<T>::has_instance_data) {
            const VkBuffer vk_instance_buffer = static_cast<VkBuffer>(renderables.template instance_buffer<T>());
            constexpr static std::size_t instance_offset = renderable_tuple<FiF, TupleT>::template static_offsets<T>()[buffer_data_type::instance];
            vkCmdBindVertexBuffers(handle, renderable_properties<T>::instance_binding(), 1, &vk_instance_buffer, &instance_offset);
        } 

        //Bind attribute buffer
        if constexpr (renderable_constraints<T>::has_attributes) {
            const VkBuffer vk_attrib_buffer = static_cast<VkBuffer>(renderables.template attribute_buffer<T>());
            constexpr static std::size_t attribute_offset = 0;
            vkCmdBindVertexBuffers(handle, renderable_properties<T>::attribute_binding(), 1, &vk_attrib_buffer, &attribute_offset);
        }

        //Bind texture index buffer
        if constexpr (renderable_constraints<T>::has_textures) {
            const VkBuffer vk_texture_idx_buffer = static_cast<VkBuffer>(renderables.template texture_idx_buffer<T>());
            const std::size_t texture_idx_offset = renderables.template texture_idx_buffer_offset<T>();
            vkCmdBindVertexBuffers(handle, renderable_properties<T>::texture_idx_binding(), 1, &vk_texture_idx_buffer, &texture_idx_offset);
        }

        //Bind index buffer
        if constexpr (renderable_constraints<T>::has_indices) {
            const VkBuffer vk_index_buffer = static_cast<VkBuffer>(renderables.template index_buffer<T>());
            const std::size_t index_offset = renderables.template index_buffer_offset<T>();
            constexpr static VkIndexType vk_index_type = static_cast<VkIndexType>(sizeof(typename renderable_properties<T>::index_type) == sizeof(std::uint32_t));
            vkCmdBindIndexBuffer(handle, vk_index_buffer, index_offset, vk_index_type);
        }

        //Bind descriptor set
        if constexpr (renderable_constraints<T>::has_uniform || renderable_constraints<T>::has_textures)
            vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, renderables.template associated_pipeline_layout<T>(), 0, renderables.template desc_set<T>().size() / FiF, renderables.template desc_set<T>().data(), 0, nullptr);

        //Set push constants
        if constexpr (renderable_constraints<T>::has_push_constants) {
            [this, &renderables]<std::size_t... Is>(std::index_sequence<Is...>) {
                constexpr static std::array pcr = T::push_constant_ranges();
                ([this, &renderables]<std::size_t I>(std::integral_constant<std::size_t, I>){
                    auto push_consts = std::get<I>(T::push_constants());
                    vkCmdPushConstants(handle, renderables.template associated_pipeline_layout<T>(), pcr[I].stageFlags, pcr[I].offset, pcr[I].size, std::addressof(push_consts));
                }(std::integral_constant<std::size_t, Is>{}), ...);
            }(std::make_index_sequence<T::push_constant_ranges().size()>{});
        }

        //Draw vertices
        //if constexpr (renderable_constraints<T>::has_fixed_vertices || renderable_constraints<T>::has_fixed_indices) {
        //    if constexpr (renderable_constraints<T>::has_indices) 
        //        vkCmdDrawIndexed(handle, renderable_properties<T>::index_count, renderables.template instance_count<T>(), 0, 0, 0);
        //    else
        //        vkCmdDraw(handle, renderable_properties<T>::vertex_count, renderables.template instance_count<T>(), 0, 0);
        //}
        //else {
        //    for(std::size_t i = 0; i < renderables.template instance_count<T>(); ++i) {
        //        if constexpr (renderable_constraints<T>::has_indices) 
        //            vkCmdDrawIndexed(handle, renderables.template index_count<T>(i), 1, renderables.template first_index<T>(i), renderables.template first_vertex<T>(i), i);
        //        else
        //            vkCmdDraw(handle, renderables.template vertex_count<T>(i), 1, renderables.template first_vertex<T>(i), i);
        //    }
        //}
        if constexpr (renderable_constraints<T>::has_indices) 
            vkCmdDrawIndexedIndirect(handle, renderables.template draw_command_buffer<T>(), 0, renderables.template draw_count<T>(), sizeof(VkDrawIndexedIndirectCommand));
        else
            vkCmdDrawIndirect(handle, renderables.template draw_command_buffer<T>(), 0, renderables.template draw_count<T>(), sizeof(VkDrawIndirectCommand));

        return {};
    }
}