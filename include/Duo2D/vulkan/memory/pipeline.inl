#pragma once
#include "Duo2D/vulkan/memory/pipeline.hpp"

#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/vulkan/memory/shader_module.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<impl::RenderableType T>
    result<pipeline<T>> pipeline<T>::create(logical_device& device, render_pass& associated_render_pass, pipeline_layout<T>& layout) noexcept {
        pipeline ret{};
        ret.dependent_handle = device;


        //Specify vertex input state
        constexpr static auto vertex_bindings = T::shader_type::binding_descs();
        constexpr static auto vertex_attributes = T::shader_type::attribute_descs();
        VkPipelineVertexInputStateCreateInfo vertex_input_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = vertex_bindings.size(),
            .pVertexBindingDescriptions = vertex_bindings.data(),
            .vertexAttributeDescriptionCount = vertex_attributes.size(),
            .pVertexAttributeDescriptions = vertex_attributes.data(),
        };

        //Specify input assembly (TEMP: specify no stripping)
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        //Specify viewport and scissor state
        constexpr static std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineViewportStateCreateInfo viewport_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };
        VkPipelineDynamicStateCreateInfo dynamic_state_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = dynamic_states.size(),
            .pDynamicStates = dynamic_states.data(),
        };

        //Create basic rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = T::cull_mode,
            .frontFace = T::front_face,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f,
        };

        //Specify multi-sampling (TEMP: no multi-sampling)
        VkPipelineMultisampleStateCreateInfo multisampling_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        //Specify No color blend
        VkPipelineColorBlendAttachmentState color_blend_attach_info{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
        VkPipelineColorBlendStateCreateInfo color_blend_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attach_info,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        };


        __D2D_TRY_MAKE(shader_module vert_shader, make<shader_module>(device, T::shader_data_type::vert, VK_SHADER_STAGE_VERTEX_BIT), tv);
        __D2D_TRY_MAKE(shader_module frag_shader, make<shader_module>(device, T::shader_data_type::frag, VK_SHADER_STAGE_FRAGMENT_BIT), tf);
        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {vert_shader.stage_info(), frag_shader.stage_info()};

        
        VkGraphicsPipelineCreateInfo pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = shader_stages.size(),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_info,
            .pRasterizationState = &rasterizer_info,
            .pMultisampleState = &multisampling_info,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = layout,
            .renderPass = associated_render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        };
        __D2D_VULKAN_VERIFY(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &ret.handle));

        return ret;
    }
}
