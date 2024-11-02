#include "Duo2D/graphics/pipeline/pipeline.hpp"

#include "Duo2D/graphics/pipeline/make.hpp"

namespace d2d {
    result<pipeline> pipeline::create(logical_device& device, std::span<VkPipelineShaderStageCreateInfo> shaders) noexcept {
        pipeline ret{};
        ret.dependent_handle = device;


        //Specify vertex input state (TEMP: specify the use of hard-coded vertex data)
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        {
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        }

        //Specify input assembly (TEMP: specify no stripping)
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
        {
        input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_info.primitiveRestartEnable = VK_FALSE;
        }

        //Specify viewport and scissor state
        VkPipelineViewportStateCreateInfo viewport_info{};
        VkPipelineDynamicStateCreateInfo dynamic_state_info{};
        constexpr static std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        {
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.viewportCount = 1;
        viewport_info.scissorCount = 1;

        dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_info.dynamicStateCount = dynamic_states.size();
        dynamic_state_info.pDynamicStates = dynamic_states.data();
        }

        //Create basic rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer_info{};
        {
        rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_info.depthClampEnable = VK_FALSE;
        rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_info.lineWidth = 1.0f;
        rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer_info.depthBiasEnable = VK_FALSE;
        rasterizer_info.depthBiasConstantFactor = 0.0f;
        rasterizer_info.depthBiasClamp = 0.0f;
        rasterizer_info.depthBiasSlopeFactor = 0.0f;
        }

        //Specify multi-sampling (TEMP: no multi-sampling)
        VkPipelineMultisampleStateCreateInfo multisampling_info{};
        { 
        multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling_info.sampleShadingEnable = VK_FALSE;
        multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        }

        //Specify No color blend
        VkPipelineColorBlendStateCreateInfo color_blend_info{};
        VkPipelineColorBlendAttachmentState color_blend_attach_info{};
        {
        color_blend_attach_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attach_info.blendEnable = VK_FALSE;
        
        color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_info.logicOpEnable = VK_FALSE;
        color_blend_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_info.attachmentCount = 1;
        color_blend_info.pAttachments = &color_blend_attach_info;
        color_blend_info.blendConstants[0] = 0.0f;
        color_blend_info.blendConstants[1] = 0.0f;
        color_blend_info.blendConstants[2] = 0.0f;
        color_blend_info.blendConstants[3] = 0.0f;
        }

        //Create pipeline layout
        __D2D_TRY_MAKE(ret.layout, make<pipeline_layout>(device), pl)


        //Create render pass
        __D2D_TRY_MAKE(ret.main_render_pass, make<render_pass>(device), rp)

        
        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = shaders.size();
        pipeline_create_info.pStages = shaders.data();
        pipeline_create_info.pVertexInputState = &vertex_input_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_info;
        pipeline_create_info.pViewportState = &viewport_info;
        pipeline_create_info.pRasterizationState = &rasterizer_info;
        pipeline_create_info.pMultisampleState = &multisampling_info;
        pipeline_create_info.pColorBlendState = &color_blend_info;
        pipeline_create_info.pDynamicState = &dynamic_state_info;
        pipeline_create_info.pDepthStencilState = nullptr;
        pipeline_create_info.pTessellationState = nullptr;
        pipeline_create_info.layout = ret.layout;
        pipeline_create_info.renderPass = ret.main_render_pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_create_info.basePipelineIndex = -1;
        __D2D_VULKAN_VERIFY(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &ret.handle));

        return ret;
    }
}
