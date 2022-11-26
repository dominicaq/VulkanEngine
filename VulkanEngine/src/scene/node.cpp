#include "node.hpp"

namespace vke {
	Node::Node(const std::string& name) : m_name{ name }, m_transform{ *this } {
		setComponent(m_transform);
	}

	void Node::setParent(Node& p) {
		m_parent = &p;
	}

	void Node::addChild(Node& child)
	{
		m_children.push_back(&child);
	}

	void Node::setComponent(Component& component)
	{
		auto it = m_components.find(component.getType());

		if (it != m_components.end()) {
			it->second = &component;
		}
		else {
			m_components.insert(std::make_pair(component.getType(), &component));
		}
	}

	Component& Node::getComponent(const std::type_index index) {
		return *m_components.at(index);
	}

	bool Node::hasComponent(const std::type_index index) {
		return m_components.count(index) > 0;
	}
}