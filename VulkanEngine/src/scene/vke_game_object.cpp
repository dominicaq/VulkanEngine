#include "vke_game_object.hpp"
#include "node.hpp"

namespace vke {
    VkeGameObject VkeGameObject::createPointLight(float lightIntesnity = 1.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f)) {
        VkeGameObject obj = VkeGameObject::createGameObject();
        obj.transform->scale.x = radius;
        obj.color = color;
        obj.pointLight = std::make_unique<PointLightComponent>();
        obj.pointLight->lightIntensity = lightIntesnity;
        return obj;
    }

    VkeGameObject VkeGameObject::createDirectionalLight(float lightIntesnity = 1.0f, float fov = 45.0f, glm::vec3 color = glm::vec3(1.0f)) {
        VkeGameObject obj = VkeGameObject::createGameObject();
        obj.color = color;
        obj.directionalLight = std::make_unique<DirectionalLightComponent>();
        obj.directionalLight->lightIntensity = lightIntesnity;
        obj.directionalLight->fov = fov;
        return obj;
    }

    VkeGameObject::VkeGameObject(id_t objId) : m_id{ objId } {
        Node dummyNode("Object");
        transform = std::make_unique<Transform>(dummyNode);
    }
}