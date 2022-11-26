#pragma once
namespace vke {
	class KeyboardInput {
	public:
		struct KeyMappings {
			int a = GLFW_KEY_A;
			int d = GLFW_KEY_D;
			int w = GLFW_KEY_W;
			int s = GLFW_KEY_S;
			int q = GLFW_KEY_Q;
			int e = GLFW_KEY_E;

			int arrowUp = GLFW_KEY_UP;
			int arrowDown = GLFW_KEY_DOWN;
			int arrowLeft = GLFW_KEY_LEFT;
			int arrowRight = GLFW_KEY_RIGHT;
		};
	};
}