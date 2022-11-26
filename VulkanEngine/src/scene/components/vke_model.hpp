#pragma once

#include "../../src/core/vke_device.hpp"
#include "../../src/core/vke_buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

#ifndef ASSET_DIR
#define ASSET_DIR "../assets/"
#endif

namespace vke {
	class VkeModel {
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal &&
					uv == other.uv;
			}
		};

		struct ModelData {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filePath);
		};

		VkeModel(VkeDevice& device, const ModelData& modelData);
		~VkeModel();

		VkeModel(const VkeModel&) = delete;
		VkeModel& operator=(const VkeModel&) = delete;
		
		static std::unique_ptr<VkeModel>createModelFromFile(VkeDevice& device, const std::string& filePath);

		void bind(VkCommandBuffer& commandBuffer);
		void draw(VkCommandBuffer& commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VkeDevice& m_device;

		std::unique_ptr<VkeBuffer> m_vertexBuffer;
		uint32_t m_vertexCount;

		bool m_hasIndexBuffer = false;
		std::unique_ptr<VkeBuffer> m_indexBuffer;
		uint32_t m_indexCount;
	};
}