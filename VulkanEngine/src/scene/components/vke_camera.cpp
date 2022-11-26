#include "vke_camera.hpp"

namespace vke {
	void VkeCamera::setOrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar) {
		this->zNear = zNear;
		this->zFar = zFar;

		glm::mat4 projectionMatrix = glm::mat4{ 1.0f };
		projectionMatrix[0][0] = 2.0f / (right - left);
		projectionMatrix[1][1] = 2.0f / (bottom - top);
		projectionMatrix[2][2] = 1.0f / (zFar - zNear);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = -zNear / (zFar - zNear);

		m_projectionMatrix = projectionMatrix;
	}

	void VkeCamera::setPespectiveProjection(float fov, float aspect, float zNear, float zFar) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		this->zNear = zNear;
		this->zFar = zFar;

		float half_fov = fov * 0.5f;
		float k = zFar / (zFar - zNear);
		float g = 1.0f / half_fov;

		m_projectionMatrix = {
			{ (1.0f / aspect) * g, 0.0f, 0.0f, 0.0f},
			{ 0.0f, g, 0.0f, 0.0f},
			{ 0.0f, 0.0f, k, 1.0f},
			{ 0.0f, 0.0f, -zNear * k, 0.0f},
		};
	}

	void VkeCamera::lookAt(glm::vec3 target, glm::vec3 up) {
		setViewDirection(target - position, up);
	}

	void VkeCamera::setViewDirection(glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		glm::mat4 viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);

		m_viewMatrix = viewMatrix;

		glm::mat4 inverseViewMatrix = glm::mat4{ 1.0f };
		inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;

		m_inverseView = inverseViewMatrix;
	}

	void VkeCamera::updateViewYXZ() {
		const float c3 = glm::cos(eulerAngles.z);
		const float s3 = glm::sin(eulerAngles.z);
		const float c2 = glm::cos(eulerAngles.x);
		const float s2 = glm::sin(eulerAngles.x);
		const float c1 = glm::cos(eulerAngles.y);
		const float s1 = glm::sin(eulerAngles.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		glm::mat4 viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);

		// Update local axis
		right = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
		up = glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
		forward = glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);

		m_viewMatrix = viewMatrix;

		glm::mat4 inverseViewMatrix = glm::mat4{ 1.0f };
		inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;

		m_inverseView = inverseViewMatrix;
	}
}