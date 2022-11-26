#include "vke_renderer.hpp"


namespace vke {
    VkeRenderer::VkeRenderer(VkeWindow& window, VkeDevice& device) : m_window{ window }, m_device{ device } {
        recreateSwapChain();
        createCommandBuffers();

        m_core.init(m_device);
        m_core.buildCoreDescriptorSets();
        m_shadowMapSystem = std::make_unique<VkeShadowMapSystem>(m_device);
        m_shadowMapSystem->initFrameBuffer();
        m_shadowMapSystem->buildShadowDescriptorSets(m_core, VkeSwapChain::MAX_FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts = m_core.getSetLayouts();

        // Init render systems
        m_shadowMapSystem->initPipeline(setLayouts);
        m_geometrySubPass = std::make_unique<GeometrySubpass>(m_device, getSwapChainRenderPass(), setLayouts);
        m_pointLightSystem = std::make_unique<PointLightSystem>(m_device, getSwapChainRenderPass(), setLayouts);
    }

    VkeRenderer::~VkeRenderer() {
        freeCommandBuffers(); 
    }

    VkCommandBuffer VkeRenderer::beginFrame() {
        assert(!m_isFrameStarted && "Can't call beginFrame when frame is not in progress");
        auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_isFrameStarted = true;
        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }

    void VkeRenderer::endFrame() {
        assert(m_isFrameStarted && "Can't call endFrame when frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_isFrameStarted = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % VkeSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VkeRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_swapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VkeRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VkeRenderer::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device.device());

        if (m_swapChain == nullptr) {
            m_swapChain = std::make_unique<VkeSwapChain>(m_device, extent);
        }
        else {
            std::shared_ptr<VkeSwapChain> oldSwapChain = std::move(m_swapChain);
            m_swapChain = std::make_unique<VkeSwapChain>(m_device, extent, oldSwapChain);
            if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void VkeRenderer::createCommandBuffers() {
        m_commandBuffers.resize(VkeSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void VkeRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }
    
    void VkeRenderer::updateDescriptorSets(FrameInfo& frameInfo) {
        uint32_t frameIndex = frameInfo.frameIndex;
        // Objects
        UniformBufferObject ubo{};
        ubo.projection = frameInfo.camera.getProjection();
        ubo.view = frameInfo.camera.getView();

        m_core.objectBuffers[frameIndex]->writeToBuffer(&ubo);
        m_core.objectBuffers[frameIndex]->flush();

        // Scene
        UniformBufferScene ubs{};
        ubs.inverseView = frameInfo.camera.getInverseView();
        m_pointLightSystem->updateDescriptors(frameInfo, ubs);
        
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;

            if (obj.directionalLight == nullptr)
                continue;

            auto rotateLight = glm::rotate(glm::mat4(1.0f), frameInfo.deltaTime, { 0.0f, -1.0f, 0.0f });
            float zNear = 1.0f;
            float zFar = 94.0f;
            glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(obj.directionalLight->fov), m_swapChain->extentAspectRatio(), zNear, zFar);
            glm::mat4 depthViewMatrix = glm::lookAt(obj.transform->translation, glm::vec3(0.0f), glm::vec3(0, 1, 0));

            obj.transform->translation = glm::vec3(rotateLight * glm::vec4(obj.transform->translation, 1.0f));
            ubs.directionalLight.position = glm::vec4(obj.transform->translation, 1.0f);
            ubs.directionalLight.viewProjection = depthProjectionMatrix * depthViewMatrix;
        }
        
        m_core.sceneBuffers[frameIndex]->writeToBuffer(&ubs);
        m_core.sceneBuffers[frameIndex]->flush();
    }

    void VkeRenderer::update(VkeCamera& activeCamera, VkeGameObject::Map& gameObjects, float dt) {
        if (auto commandBuffer = beginFrame()) {
            int frameIndex = getFrameIndex();
            float aspectRatio = getAspectRatio();
            activeCamera.setPespectiveProjection(glm::radians(90.0f), aspectRatio, 0.01f, 1000.0f);
            activeCamera.updateViewYXZ();

            FrameInfo frameInfo = { frameIndex, dt, commandBuffer, activeCamera, m_core.getSets(frameIndex), gameObjects};
            updateDescriptorSets(frameInfo);

            assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
            assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");
            
            // Shadow render pass
            m_shadowMapSystem->render(frameInfo);

            // Main render pass
            beginSwapChainRenderPass(commandBuffer);
            m_geometrySubPass->draw(frameInfo);
            m_pointLightSystem->render(frameInfo);
            endSwapChainRenderPass(frameInfo.commandBuffer);

            endFrame();
        }
    }
}