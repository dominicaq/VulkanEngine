#pragma once

#include "../core/vke_device.hpp"
#include "../core/vke_pipeline.hpp"
#include "../core/vke_frame_info.hpp"
#include "../scene/vke_game_object.hpp"
#include "../scene/components/vke_camera.hpp"

// std
#include <memory>
#include <vector>
#include <array>
#include <stdexcept>

#define MAX_LIGHTS 10

namespace vke {
	class PointLightSystem {
	public:
		PointLightSystem(VkeDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void updateDescriptors(FrameInfo& frameInfo, UniformBufferScene& ubs);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
		void createPipeline(VkRenderPass renderPass);

		VkeDevice& m_device;
		std::unique_ptr<VkePipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;

		struct PointLight {
			glm::vec4 position{};
			glm::vec4 color{};
		};

		struct DirectionalLight {
			glm::vec4 position{};
			glm::mat4 viewProjection{ 1.0f };
		};

		struct UniformBufferLights {
			glm::mat4 inverseView{ 1.0f };
			glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
			PointLight pointLights[MAX_LIGHTS];
			DirectionalLight directionalLight;
			int numLights;
		}m_UBL;

		std::unique_ptr<VkeDescriptorPool>		m_lightPool;
		std::unique_ptr<VkeDescriptorSetLayout> m_lightLayout;
		std::vector<VkDescriptorSet>			m_lightSet;
		std::vector<std::unique_ptr<VkeBuffer>> m_lightBuffers;
	};
}