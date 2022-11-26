#include "component.hpp"
#include <algorithm>

#include "../node.hpp"

namespace vke {
	Component::Component(const std::string& name) : m_name{ name } {

	}

	const std::string& Component::getName() const {
		return m_name;
	}
}