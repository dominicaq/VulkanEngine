#include "vke_window.hpp"

namespace vke {
	VkeWindow::VkeWindow(int w, int h, std::string name) : m_width{ w }, m_height{ h }, m_name{ name }{
		initWindow();
	}

	VkeWindow::~VkeWindow() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void VkeWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}

	void VkeWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VkeWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto vkeWindow = reinterpret_cast<VkeWindow*>(glfwGetWindowUserPointer(window));
		vkeWindow->m_frameBufferResized = true;
		vkeWindow->m_width = width;
		vkeWindow->m_height = height;
	}
}
