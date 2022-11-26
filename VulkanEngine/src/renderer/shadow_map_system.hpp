#pragma once
// REFERENCE MATERIAL: https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp
#include "../core/vke_pipeline.hpp"
#include "../core/vke_frame_info.hpp"
#include "../core/vke_descriptors.hpp"
#include "../core/vke_frame_buffer.hpp"
#include "../core/vke_core.hpp"
#include <array>

// Offscreen frame buffer properties
#define DEFAULT_SHADOWMAP_FILTER VK_FILTER_LINEAR
#define DEPTH_FORMAT VK_FORMAT_D16_UNORM
#define SHADOWMAP_DIM 2048

namespace vke {
	class VkeShadowMapSystem {
	public:
		VkeShadowMapSystem(VkeDevice& device);
		~VkeShadowMapSystem();

		void render(FrameInfo& frameInfo);
		void initPipeline(std::vector<VkDescriptorSetLayout>& setLayouts);
		void initFrameBuffer();
		VkDescriptorImageInfo getFrameBufferImageInfo();
		void buildShadowDescriptorSets(VkeCore& core, uint32_t framesInFlight);
		
		const float depthBiasConstant = 1.25f;
		const float depthBiasClamp = 0.0f;
		const float depthBiasSlope = 3.75f;
	private:
		void createPipeline(VkRenderPass renderPass);
		void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);

		VkeDevice& m_device;
		VkPipelineLayout m_pipelineLayout;
		std::unique_ptr<VkePipeline> m_pipeline;
		std::unique_ptr<VkeFrameBuffer> m_frameBuffer;
	};
}