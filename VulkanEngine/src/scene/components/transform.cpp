#include "transform.hpp"

#include "../node.hpp"

namespace vke {
    Transform::Transform(Node& node) : m_node{ node } { }

    std::type_index Transform::getType() {
        return typeid(Transform);
    }

    void Transform::setRotation(const glm::quat& newRotation) {
        m_rotation = newRotation;
        m_eulerAngles = glm::eulerAngles(newRotation);
    }

    void Transform::setEulerAngles(const glm::vec3& newAngles) {
        m_eulerAngles = newAngles;
        m_rotation = toQuaternion(newAngles);
    }

    glm::mat4 Transform::getModelMatrix() {
        updateMatrices();
        return m_modelMatrix;
    }

    glm::mat4 Transform::getNormalMatrix() {
        updateMatrices();
        return m_normalMatrix;
    }

    void Transform::updateMatrices() {
        glm::quat rot = m_rotation;
        m_modelMatrix = glm::translate(glm::mat4(1.0), translation) *
            glm::mat4_cast(rot) *
            glm::scale(glm::mat4(1.0), scale);

        m_normalMatrix = glm::translate(glm::mat4(1.0), translation) *
            glm::mat4_cast(rot) *
            glm::scale(glm::mat4(1.0), 1.0f / scale);

        auto parent = m_node.getParent();

        if (parent) {
            auto& parentTransform = parent->getComponent<Transform>();
            m_modelMatrix = m_modelMatrix * parentTransform.getModelMatrix();
            m_normalMatrix = m_normalMatrix * parentTransform.getNormalMatrix();
        }
    }

    glm::quat Transform::toQuaternion(glm::vec3 angles) {
        double cy = glm::cos(angles.x * 0.5);
        double sy = glm::sin(angles.x * 0.5);
        double cp = glm::cos(angles.y * 0.5);
        double sp = glm::sin(angles.y * 0.5);
        double cr = glm::cos(angles.z * 0.5);
        double sr = glm::sin(angles.z * 0.5);

        glm::quat q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
    }
}
