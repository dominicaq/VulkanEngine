#pragma once

#include "../scene/components/vke_model.hpp"
#include "../scene/components/transform.hpp"

// std
#include <memory>
#include <unordered_map>

// https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers/tree/master/framework/scene_graph
namespace vke {
	struct PointLightComponent {
		float lightIntensity = 1.0f;
		float radius = 1.0f;
	};

	struct DirectionalLightComponent {
		float lightIntensity = 1.0f;
		float fov = 45.0f;
	};

	class VkeGameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VkeGameObject>;

		static VkeGameObject createGameObject() {
			static id_t currentId = 0;
			return VkeGameObject{ currentId++ };
		}

		static VkeGameObject createPointLight(float lightIntesnity, float radius, glm::vec3 color);
		static VkeGameObject createDirectionalLight(float lightIntesnity, float fov, glm::vec3 color);

		VkeGameObject(const VkeGameObject&) = delete;
		VkeGameObject& operator=(const VkeGameObject&) = delete;
		VkeGameObject(VkeGameObject&&) = default;
		VkeGameObject& operator=(VkeGameObject&&) = default;

		id_t getId() { return m_id; }

		std::shared_ptr<Transform> transform;
		glm::vec3 color{};

		std::shared_ptr<VkeModel> model{};

		std::shared_ptr<PointLightComponent> pointLight = nullptr;
		std::shared_ptr<DirectionalLightComponent> directionalLight = nullptr;

	private:
		VkeGameObject(id_t objId);
		std::string m_name;
		id_t m_id;
	};
}