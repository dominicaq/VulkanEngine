#pragma once

#include "../core/vke_device.hpp"
#include "../core/vke_pipeline.hpp"
#include "../core/vke_frame_info.hpp"
#include "../core/vke_swap_chain.hpp"
#include "../scene/components/vke_camera.hpp"
#include "../scene/vke_game_object.hpp"

// std
#include <memory>
#include <vector>
#include <array>
#include <stdexcept>

namespace vke {
	struct alignas(16) GlobalUniform {
		glm::mat4 model;
		glm::mat4 viewProjection;
		glm::vec3 cameraPosition;
	};

	class GeometrySubpass {
	public:
		GeometrySubpass(VkeDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
		~GeometrySubpass();

		GeometrySubpass(const GeometrySubpass&) = delete;
		GeometrySubpass& operator=(const GeometrySubpass&) = delete;
		void draw(FrameInfo& frameInfo);

		void updateUniform(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
		void createPipeline(VkRenderPass renderPass);

		VkeDevice& m_device;
		std::unique_ptr<VkePipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};
}