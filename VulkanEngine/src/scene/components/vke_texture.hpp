#pragma once

#include "../../core/vke_buffer.hpp"
#include "../../core/vke_descriptors.hpp"

#ifndef ASSET_DIR
#define ASSET_DIR "../assets/"
#endif

namespace vke {
	class VkeTexture {
	public:
		using id_t = unsigned int;
		struct TextureData {
			VkSampler      sampler;
			VkImage        image;
			VkImageLayout  imageLayout;
			VkDeviceMemory memory;
			VkImageView    view;
		};

		static std::unique_ptr<VkeTexture> createTexture(VkeDevice& device, const std::string& filePath) {
			static id_t currentId = 0;
			return std::make_unique<VkeTexture>(device, filePath, currentId++);
		}
		VkeTexture(VkeDevice& device, const std::string& filePath, id_t texId);
		~VkeTexture();

		VkDescriptorImageInfo getDescriptorImageInfo();
		
		// Helper functions
		void setFormat(VkFormat inFormat) { m_format = inFormat; }
		bool isReady() { return m_ready; }
		VkFormat getFormat() { return m_format; }
		int32_t getWidth() { return m_width; }
		int32_t getHeight() { return m_height; }
		id_t getId() { return m_id; }
	private:
		void createTextureImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProperties);
		void createTextureImageView();
		void createTextureSampler();
		void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

		VkeDevice& m_device;

		int32_t m_width;
		int32_t m_height;
		bool    m_ready;

		VkImageTiling     m_tiling;
		VkImageUsageFlags m_usage_flags;
		VkFlags           m_memory_flags;
		VkFormat          m_format;
		VkImage			  m_image;

		TextureData m_data;
		uint32_t	m_mip_level;

		id_t		m_id;
	};
}
