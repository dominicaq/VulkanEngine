#pragma once

#include "../vke_window.hpp"
#include "../core/vke_device.hpp"
#include "../core/vke_swap_chain.hpp"
#include "../core/vke_core.hpp"
#include "../scene/vke_game_object.hpp"

// Systems
#include "../renderer/geometry_subpass.hpp"
#include "../renderer/point_light_system.hpp"
#include "../renderer/shadow_map_system.hpp"

// std
#include <array>
#include <memory>
#include <vector>
#include <cassert>
#include <array>

namespace vke {
	class VkeRenderer {
	public:
		VkeRenderer(VkeWindow& window, VkeDevice& device);
		~VkeRenderer();

		VkeRenderer(const VkeRenderer&) = delete;
		VkeRenderer& operator=(const VkeRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();

		void update(VkeCamera& activeCamera, VkeGameObject::Map& gameObjects, float dt);
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		// Helper functions
		float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return m_isFrameStarted; }
		int getFrameIndex() const {
			assert(m_isFrameStarted && "Cannot get frame index when frame is not in progress");
			return m_currentFrameIndex;
		}
		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_commandBuffers[m_currentFrameIndex];
		}
		VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void updateDescriptorSets(FrameInfo& frameInfo);

		VkeWindow& m_window;
		VkeDevice& m_device;

		std::unique_ptr<VkeSwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		// Descriptor heap
		VkeCore m_core;

		// Render
		std::unique_ptr<VkeShadowMapSystem> m_shadowMapSystem;
		std::unique_ptr<GeometrySubpass> m_geometrySubPass;
		std::unique_ptr<PointLightSystem> m_pointLightSystem;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex{ 0 };
		bool m_isFrameStarted = false;
	};
}