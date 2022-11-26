#include "scene_graph.hpp"
#include "components/vke_texture.hpp"

#include <queue>
#include <cassert>

namespace vke {
	Scene::Scene(const std::string& name) : m_name{ name } { }

	void Scene::setNodes(std::vector<std::unique_ptr<Node>>&& node) {
		assert(m_nodes.empty() && "Scene nodes were already set");
		m_nodes = std::move(node);
	}

	void Scene::addNode(std::unique_ptr<Node>&& node) {
		m_nodes.emplace_back(std::move(node));
	}

	void Scene::addChild(Node& child) {
		m_root->addChild(child);
	}

	void Scene::addComponent(std::unique_ptr<Component>&& component, Node& node) {
		node.setComponent(*component);

		if (component) {
			m_components[component->getType()].push_back(std::move(component));
		}
	}

	void Scene::addComponent(std::unique_ptr<Component>&& component) {
		if (component) {
			m_components[component->getType()].push_back(std::move(component));
		}
	}

	void Scene::setComponents(const std::type_index& typeInfo, std::vector<std::unique_ptr<Component>>&& newComponents) {
		m_components[typeInfo] = std::move(newComponents);
	}

	const std::vector<std::unique_ptr<Component>>& Scene::getComponents(const std::type_index& typeInfo) const {
		return m_components.at(typeInfo);
	}

	bool Scene::hasComponent(const std::type_index& type_info) const {
		auto component = m_components.find(type_info);
		return (component != m_components.end() && !component->second.empty());
	}

	Node* Scene::findNode(const std::string& nodeName) {
		for (auto rootNode : m_root->getChildren()) {
			std::queue<Node*> traverseNodes{};
			traverseNodes.push(rootNode);

			while (!traverseNodes.empty()) {
				auto node = traverseNodes.front();
				traverseNodes.pop();

				if (node->getName() == nodeName)
					return node;

				for (auto child_node : node->getChildren()) {
					traverseNodes.push(child_node);
				}
			}
		}

		return nullptr;
	}

	void Scene::setRootNode(Node& node) {
		m_root = &node;
	}

	Node& Scene::getRootNode() {
		return *m_root;
	}
}
