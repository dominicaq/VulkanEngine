#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "components/transform.hpp"
#include "components/component.hpp"

namespace vke {
	class Component;
	
	class Node {
	public:
		Node(const std::string& name);
		virtual ~Node() = default;

		const std::string& getName() const { return m_name; }
		Transform& getTransform() { return m_transform; }

		void setParent(Node& parent);
		Node* getParent() const { return m_parent; };

		void addChild(Node& child);
		const std::vector<Node*>& getChildren() const { return m_children; }

		void setComponent(Component& component);

		template <class T>
		inline T& getComponent() { return dynamic_cast<T&>(getComponent(typeid(T))); }
		Component& getComponent(const std::type_index index);

		template <class T>
		bool hasComponent() { return hasComponent(typeid(T)); }
		bool hasComponent(const std::type_index index);

		const std::string getName() { return m_name; }
	private:
		std::string m_name;
		Transform m_transform;
		std::unordered_map<std::type_index, Component*> m_components;

		Node* m_parent{ nullptr };
		std::vector<Node*> m_children;
	};
}
