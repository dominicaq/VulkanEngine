#pragma once

#include "component.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vke {
	class Node;
	class Transform : public Component {
	public:
		Transform(Node& node);
		Transform();
		virtual ~Transform() = default;

		Node& getNode() { return m_node; }
		virtual std::type_index getType() override;
		
		glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		void setEulerAngles(const glm::vec3& angles);
		void setRotation(const glm::quat& q);

		glm::quat getRotation() { return m_rotation; }
		glm::vec3 getEulerAngles() { return m_eulerAngles; }

		// Math
		glm::mat4 getModelMatrix();
		glm::mat4 getNormalMatrix();
	private:
		void updateMatrices();
		glm::quat toQuaternion(glm::vec3 angle);

		Node& m_node;

		glm::quat m_rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 m_eulerAngles{ 0.0f, 0.0f,0.0f };

		glm::mat4 m_modelMatrix = glm::mat4(1.0f);
		glm::mat4 m_normalMatrix = glm::mat4(1.0f);
		bool m_updateMatrices = false;
	};
}