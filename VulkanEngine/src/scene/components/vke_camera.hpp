#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke {
	class VkeCamera {
	public:
		float zNear = 0.0f;
		float zFar = 0.0f;

		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 eulerAngles{ 0.0f, 0.0f, 0.0f };
		
		// Axis orientation
		glm::vec3 right{ 1.0f, 0.0f, 0.0f }, up{ 0.0f, -1.0f, 0.0f }, forward{ 0.0f, 0.0f, 1.0f };

		// Helper functions
		void setOrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar);
		void setPespectiveProjection(float fov, float aspect, float zNear, float zFar);

		void lookAt(glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
		void setViewDirection(glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});		

		void updateViewYXZ();
		const glm::mat4& getProjection() const { return m_projectionMatrix; }
		const glm::mat4& getView() const { return m_viewMatrix; }
		const glm::mat4& getInverseView() const { return m_inverseView; }

	private:
		glm::mat4 m_projectionMatrix{ 1.0f };
		glm::mat4 m_viewMatrix{ 1.0f };
		glm::mat4 m_inverseView{ 1.0f };
	};
}