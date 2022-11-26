#pragma once
#include "shadow_map_system.hpp"
#include <array>

namespace vke {
    struct PushConstantData {
        glm::mat4 modelMatrix{ 1.0f };
        glm::mat4 normalMatrix{ 1.0f };
    };

    VkeShadowMapSystem::VkeShadowMapSystem(VkeDevice& device) : m_device{ device } { }

    VkeShadowMapSystem::~VkeShadowMapSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }
    
    void VkeShadowMapSystem::buildShadowDescriptorSets(VkeCore& core, uint32_t framesInFlight) {
        auto shadowImage = getFrameBufferImageInfo();
        for (int i = 0; i < (int)framesInFlight; i++) {
            VkeDescriptorWriter(*core.shadowSetLayout, *core.shadowDescriptorPool)
                .writeImage(0, &shadowImage)
                .build(core.shadowSet[i]);
        }

        core.descriptorSets[1] = core.shadowSet;
    }

    void VkeShadowMapSystem::initFrameBuffer() {
        m_frameBuffer = std::make_unique<VkeFrameBuffer>(m_device);
        m_frameBuffer->width = SHADOWMAP_DIM;
        m_frameBuffer->height = SHADOWMAP_DIM;

        VkFilter shadowmap_filter = m_device.formatIsFilterable(DEPTH_FORMAT, VK_IMAGE_TILING_OPTIMAL) ?
            VK_FILTER_LINEAR :
            VK_FILTER_NEAREST;

        if (m_frameBuffer->createSampler(shadowmap_filter, shadowmap_filter, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth sampler!");
        }

        AttachmentCreateInfo depthAttachment{};
        depthAttachment.format = DEPTH_FORMAT;
        depthAttachment.width = m_frameBuffer->width;
        depthAttachment.height = m_frameBuffer->height;
        depthAttachment.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depthAttachment.imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.layerCount = 1;
        m_frameBuffer->addAttachment(depthAttachment);
        m_frameBuffer->createRenderPass();
    }

    void VkeShadowMapSystem::render(FrameInfo& frameInfo) {
        beginRenderPass(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(frameInfo.descriptorSets.size()),
            frameInfo.descriptorSets.data(),
            0,
            nullptr);

        m_pipeline->bind(frameInfo.commandBuffer);
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;

            if (obj.model == nullptr)
                continue;

            PushConstantData push{};
            push.modelMatrix = obj.transform->getModelMatrix();;
            push.normalMatrix = obj.transform->getNormalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstantData),
                &push);

            // Draw desired objects for depth attachment update
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }

        endRenderPass(frameInfo.commandBuffer);
    }

    void VkeShadowMapSystem::beginRenderPass(VkCommandBuffer commandBuffer) {
        VkExtent2D extent{};
        extent.width = m_frameBuffer->width;
        extent.height = m_frameBuffer->height;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_frameBuffer->renderPass;
        renderPassInfo.framebuffer = m_frameBuffer->framebuffer;
        renderPassInfo.renderArea.extent = extent;

        VkClearValue clearValues[2];
        clearValues[0].depthStencil = { 1.0f, 0 };
        renderPassInfo.pClearValues = clearValues;
        renderPassInfo.clearValueCount = 1;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{ {0, 0}, extent };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdSetDepthBias(commandBuffer, depthBiasConstant, depthBiasClamp, depthBiasSlope);
    }

    void VkeShadowMapSystem::endRenderPass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    VkDescriptorImageInfo VkeShadowMapSystem::getFrameBufferImageInfo() {
        VkDescriptorImageInfo descriptorImageInfo{};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView = m_frameBuffer->attachments[0].view;
        descriptorImageInfo.sampler = m_frameBuffer->sampler;
        return descriptorImageInfo;
    }

    void VkeShadowMapSystem::initPipeline(std::vector<VkDescriptorSetLayout>& setLayouts) {
        createPipelineLayout(setLayouts);
        createPipeline(m_frameBuffer->renderPass);
    }

    void VkeShadowMapSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    
    void VkeShadowMapSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        VkePipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.colorBlendInfo.attachmentCount = 0;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;

        pipelineConfig.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        pipelineConfig.dynamicStateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
        pipelineConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.dynamicStateEnables.size());
        pipelineConfig.dynamicStateInfo.flags = 0;

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VkePipeline>(
            m_device,
            "VulkanEngine/src/shaders/shadow.vert.spv",
            "VulkanEngine/src/shaders/blank.frag.spv",
            pipelineConfig);
    }
}