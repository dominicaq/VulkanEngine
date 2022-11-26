#pragma once

#include "../scene/components/vke_camera.hpp"
#include "../scene/vke_game_object.hpp"
#include "vke_core.hpp"

#include <array>

//lib 
#include <vulkan/vulkan.h>

namespace vke {
#define MAX_LIGHTS 10
	struct PointLight {
		glm::vec4 position{};
		glm::vec4 color{};
	};

	struct DirectionalLight {
		glm::vec4 position{};
		glm::mat4 viewProjection{ 1.0f };
	};
	
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 modelNormal;
	};
	
	struct UniformBufferScene {
		glm::mat4 inverseView{ 1.0f };
		glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
		PointLight pointLights[MAX_LIGHTS];
		DirectionalLight directionalLight;
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float deltaTime;
		VkCommandBuffer commandBuffer;

		// Scene
		VkeCamera& camera;
		std::vector<VkDescriptorSet> descriptorSets;
		VkeGameObject::Map& gameObjects;
	};
}