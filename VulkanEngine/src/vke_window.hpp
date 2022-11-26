#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <string>
#include <stdexcept>

namespace vke {
	class VkeWindow {
	public:
		VkeWindow(int width, int height, std::string name);
		~VkeWindow();

		VkeWindow(const VkeWindow&) = delete;
		VkeWindow& operator=(const VkeWindow&) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		
		// Helper functions
		GLFWwindow* getGLFWwindow() const { return m_window; }
		bool shouldClose() const { return glfwWindowShouldClose(m_window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }
		bool wasWindowResized() { return m_frameBufferResized; }
		void resetWindowResizedFlag() { m_frameBufferResized = false; }

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		bool m_frameBufferResized = false;
		int m_width, m_height;
		std::string m_name;
		GLFWwindow* m_window;
	};
}