#include "geometry_subpass.hpp"

namespace vke {
    struct PushModelData {
        glm::mat4 modelMatrix{ 1.0f };
        glm::mat4 normalMatrix{ 1.0f };
    };

    GeometrySubpass::GeometrySubpass(VkeDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) : m_device { device } {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);
    }

    GeometrySubpass::~GeometrySubpass() { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }
    
    void GeometrySubpass::draw(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(frameInfo.descriptorSets.size()),
            frameInfo.descriptorSets.data(),
            0,
            nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;

            if (obj.model == nullptr)
                continue;
         
            PushModelData push{};
            push.modelMatrix = obj.transform->getModelMatrix();
            push.normalMatrix = obj.transform->getNormalMatrix();
            
            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushModelData),
                &push);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void GeometrySubpass::updateUniform(FrameInfo& frameInfo) {
        GlobalUniform globalUniform{};
        globalUniform.viewProjection = frameInfo.camera.getProjection() * frameInfo.camera.getView();
    }

    void GeometrySubpass::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        VkePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VkePipeline>(
            m_device,
            "VulkanEngine/src/shaders/simple_shader.vert.spv",
            "VulkanEngine/src/shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void GeometrySubpass::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushModelData);

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
}