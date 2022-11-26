#include "point_light_system.hpp"

#include <map>

namespace vke {
    struct PointLightPushConstant {
        glm::vec4 posiiton;
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(VkeDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) : m_device{ device } {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }

    void PointLightSystem::updateDescriptors(FrameInfo& frameInfo, UniformBufferScene& ubs) {
        int lightIndex = 0;
        auto rotateLight = glm::rotate(glm::mat4(1.0f), frameInfo.deltaTime, { 0.0f, -1.0f, 0.0f });

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr)
                continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceeding maximum!");

            // update (temp)
            //obj.transform->setTranslation(glm::vec3(rotateLight * glm::vec4(obj.transform->getTranslation(), 1.0f)));
            obj.transform->translation = glm::vec3(rotateLight * glm::vec4(obj.transform->translation, 1.0f));

            ubs.pointLights[lightIndex].position = glm::vec4(obj.transform->translation, 1.0f);
            ubs.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            lightIndex += 1;
        }
        
        ubs.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo& frameInfo) {
        // light sorting (temp) for point light halo
        std::map<float, VkeGameObject::id_t> sorted;
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            // distance calculation
            auto offset = frameInfo.camera.position - obj.transform->translation;
            float distSquared = glm::dot(offset, offset);
            sorted[distSquared] = obj.getId();
        }

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

        // Iterate through sorted lights in reverse order
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
            auto& obj = frameInfo.gameObjects.at(it->second);

            PointLightPushConstant push{};
            push.posiiton = glm::vec4(obj.transform->translation, 1.0f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform->scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstant),
                &push
                );

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        VkePipeline::defaultPipelineConfigInfo(pipelineConfig);
        VkePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VkePipeline>(
            m_device,
            "VulkanEngine/src/shaders/point_light.vert.spv",
            "VulkanEngine/src/shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstant);

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