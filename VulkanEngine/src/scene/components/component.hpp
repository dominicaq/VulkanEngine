#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <vector>
#include <algorithm>

namespace vke {
	class Component {
	public:
		Component() = default;
		Component(const std::string& name);
		Component(Component&& other) = default;

		virtual ~Component() = default;
		virtual std::type_index getType() = 0;
		const std::string& getName() const;
	private:
		std::string m_name;
	};
}