#pragma once

#include "vke_window.hpp"
#include "core/vke_device.hpp"
#include "renderer/vke_renderer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
#include <array>
#include <stdexcept>

namespace vke {
	class VkeApplication {
	public:
		VkeApplication();
		~VkeApplication();

		VkeApplication(const VkeApplication&) = delete;
		VkeApplication& operator=(const VkeApplication&) = delete;

		void run();

	private:
		void loadGameObjects();

		VkeWindow m_window{ 1000, 1000, "Vulkan Renderer" };
		VkeDevice m_device{ m_window };
		VkeRenderer m_renderer{ m_window, m_device };

		float m_lastFrameTime = 0.0f;
		VkeGameObject::Map m_gameObjects;
	};
}