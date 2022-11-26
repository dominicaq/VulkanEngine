#include "vke_application.hpp"
#include "timer.hpp"
#include "timestep.hpp"
#include "user_input.hpp"

// Scene
#include "scene/components/vke_camera.hpp"
#include "scene/vke_game_object.hpp"
#include "scene/scene_graph.hpp"

// AXIS USED:
// Z+ = forward
// Y- = Up

namespace vke {  
    void CameraController(GLFWwindow* window, float dt, VkeCamera& camera) {
        float lookSpeed = 1.0f;
        float moveSpeed = 2.0f;
        KeyboardInput::KeyMappings input;

        glm::vec3 eulerAngles{0.0f};
        if (glfwGetKey(window, input.arrowRight) == GLFW_PRESS) { eulerAngles.y += 1.0f; }
        if (glfwGetKey(window, input.arrowLeft) == GLFW_PRESS) { eulerAngles.y -= 1.0f; }
        if (glfwGetKey(window, input.arrowUp) == GLFW_PRESS) { eulerAngles.x += 1.0f; }
        if (glfwGetKey(window, input.arrowDown) == GLFW_PRESS) { eulerAngles.x -= 1.0f; }

        if (glm::dot(eulerAngles, eulerAngles) > std::numeric_limits<float>::epsilon()) {
            camera.eulerAngles += glm::normalize(eulerAngles) * lookSpeed * dt;
        }
        camera.eulerAngles.x = glm::clamp(camera.eulerAngles.x, -glm::pi<float>()/2.0f, glm::pi<float>()/2.0f);
        
        glm::vec3 moveDirection{ 0.0f };
        if (glfwGetKey(window, input.d) == GLFW_PRESS) { moveDirection.x += 1.0f; }
        if (glfwGetKey(window, input.a) == GLFW_PRESS) { moveDirection.x -= 1.0f; }
        if (glfwGetKey(window, input.w) == GLFW_PRESS) { moveDirection.z += 1.0f; }
        if (glfwGetKey(window, input.s) == GLFW_PRESS) { moveDirection.z -= 1.0f; }

        if (glfwGetKey(window, input.q) == GLFW_PRESS) { moveDirection.y -= 1.0f; }
        if (glfwGetKey(window, input.e) == GLFW_PRESS) { moveDirection.y += 1.0f; }

        if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon()) {
            camera.position += glm::normalize(moveDirection) * moveSpeed * dt;
        }
    }

    VkeApplication::VkeApplication() {
        loadGameObjects();
    }

    VkeApplication::~VkeApplication() { }

	void VkeApplication::run() {
        VkeCamera sceneCamera{};
        sceneCamera.position = glm::vec3(0.0f, -1.0f, -4.0f);
        
		while (!m_window.shouldClose()) {
            glfwPollEvents();
            
            float time = glfwGetTime();
            TimeStep deltaTime = time - m_lastFrameTime;
            m_lastFrameTime = time;
            
            CameraController(m_window.getGLFWwindow(), deltaTime, sceneCamera);

            m_renderer.update(sceneCamera, m_gameObjects, deltaTime);
		}

        vkDeviceWaitIdle(m_device.device());
	}

    void VkeApplication::loadGameObjects() {
        // New system
        //Scene scene("default scene");

        //auto gameObject = VkeGameObject::createGameObject();
        //scene.addNode(gameObject);
        
        // end of new system
        // GameObjects
        std::shared_ptr<VkeModel> torusModel = VkeModel::createModelFromFile(m_device, "models/torus.obj");
        auto torus = VkeGameObject::createGameObject();
        torus.model = torusModel;
        torus.transform->translation = { 0.0f, -0.5f, 0.0f };
        torus.transform->scale = glm::vec3{ 1.5f };
        //m_gameObjects.emplace(torus.getId(), std::move(torus));

        std::shared_ptr<VkeModel> vaseModel = VkeModel::createModelFromFile(m_device, "models/smooth_vase.obj");
        //std::shared_ptr<VkeModel> armadilloModel = VkeModel::createModelFromFile(m_device, "models/armadillo.obj");
        //std::shared_ptr<VkeTexture> defaultTexture = VkeTexture::createTexture(m_device, "textures/checkerboard.jpg");
        auto centerObject = VkeGameObject::createGameObject();
        centerObject.model = vaseModel;
        centerObject.transform->translation = { 0.0f, 0.5f, 0.0f };
        centerObject.transform->scale = glm::vec3{ 3.0f };
        m_gameObjects.emplace(centerObject.getId(), std::move(centerObject));   

        std::shared_ptr<VkeModel> quadModel = VkeModel::createModelFromFile(m_device, "models/quad.obj");
        auto quad = VkeGameObject::createGameObject();
        quad.model = quadModel;
        quad.transform->translation = { 0.0f, 0.5f, 0.0f };
        quad.transform->scale = glm::vec3{ 10.0f };
        m_gameObjects.emplace(quad.getId(), std::move(quad));

        // Point Lights
        std::vector<glm::vec3> lightColors{
             {1.f, 1.f, 1.f},
             {.1f, .1f, 1.f}
        };
        
        auto directionalLight = VkeGameObject::createDirectionalLight(0.9f, 45.0f, lightColors[1]);
        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = VkeGameObject::createPointLight(0.9f, 0.1f, lightColors[i]);
            auto rotateLight = glm::rotate(
                glm::mat4(1.0f),
                i * glm::two_pi<float>() / lightColors.size(),
                { 0.0f, -1.0f, 0.0f }
            );

            pointLight.transform->translation = glm::vec3(rotateLight * glm::vec4(-2.0f, -1.0f, -2.0f, 1.0f));
            m_gameObjects.emplace(pointLight.getId(), std::move(pointLight));

            if (i > 0)
                continue;

            directionalLight.transform->translation = glm::vec3(rotateLight * glm::vec4(-2.0f, -1.0f, -2.0f, 1.0f));
            directionalLight.directionalLight->fov = 90.0f;
            m_gameObjects.emplace(directionalLight.getId(), std::move(directionalLight));
        }
    }
}