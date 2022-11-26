#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "node.hpp"

class Node;
class Component;

namespace vke {
	class Scene {
	public:
		Scene() = default;
		Scene(const std::string& name);

		void setName(const std::string& name) { m_name = name; }

		void setNodes(std::vector<std::unique_ptr<Node>>&& nodes);
		void addNode(std::unique_ptr<Node>&& node);
		void addChild(Node& child);
		void addComponent(std::unique_ptr<Component>&& component);
		void addComponent(std::unique_ptr<Component>&& component, Node& node);

		/**
		* @brief Set list of components for the given type
		* @param type_info The type of the component
		* @param components The list of components (retained)
		*/
		void setComponents(const std::type_index& type_info, std::vector<std::unique_ptr<Component>>&& components);

		/**
		* @brief Set list of components casted from the given template type
		*/
		template <class T>
		void setComponents(std::vector<std::unique_ptr<T>>&& components) {
			std::vector<std::unique_ptr<Component>> result(components.size());
			std::transform(components.begin(), components.end(), result.begin(),
				[](std::unique_ptr<T>& component) -> std::unique_ptr<Component> {
					return std::unique_ptr<Component>(std::move(component));
				});
			setComponents(typeid(T), std::move(result));
		}

		/**
		* @brief Clears a list of components
		*/
		template <class T>
		void clearComponents() {
			setComponents(typeid(T), {});
		}

		/**
		* @return List of pointers to components casted to the given template type
		*/
		template <class T>
		std::vector<T*> getComponents() const
		{
			std::vector<T*> result;
			if (hasComponents(typeid(T)))
			{
				auto& scene_components = getComponents(typeid(T));

				result.resize(scene_components.size());
				std::transform(scene_components.begin(), scene_components.end(), result.begin(),
					[](const std::unique_ptr<Component>& component) -> T* {
						return dynamic_cast<T*>(component.get());
					});
			}

			return result;
		}

		/**
		* @return List of components for the given type
		*/
		const std::vector<std::unique_ptr<Component>>& getComponents(const std::type_index& type_info) const;

		template <class T>
		bool hasComponents() const {
			return hasComponents(typeid(T));
		}

		bool hasComponent(const std::type_index& type_info) const;

		Node* findNode(const std::string& name);
		void setRootNode(Node& node);
		Node& getRootNode();
	private:
		std::string m_name;

		/// List of all the nodes
		std::vector<std::unique_ptr<Node>> m_nodes;
		Node* m_root{ nullptr };
		std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> m_components;
	};
}