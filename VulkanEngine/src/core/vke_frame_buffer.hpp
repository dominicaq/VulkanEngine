#pragma once
#include "vke_device.hpp"

#include <array>
#include <vector>
#include <cassert>

#define VK_FLAGS_NONE 0

namespace vke {
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkFormat format;
		VkImageSubresourceRange subReourceRange;
		VkAttachmentDescription description;

		bool hasDepth() {
			std::vector<VkFormat> formats =
			{
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		bool hasStencil() {
			std::vector<VkFormat> formats =
			{
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		bool isDepthStencil() {
			return(hasDepth() || hasStencil());
		}
	};

	struct AttachmentCreateInfo {
		uint32_t width, height;
		uint32_t layerCount;
		VkFormat format;
		VkImageUsageFlags usage;
		VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
	};

	class VkeFrameBuffer {
	public:
		VkeFrameBuffer(VkeDevice& device);
		~VkeFrameBuffer();

		VkeFrameBuffer(const VkeFrameBuffer&) = delete;
		VkeFrameBuffer& operator=(const VkeFrameBuffer&) = delete;
		VkeFrameBuffer(VkeFrameBuffer&&) = delete;
		VkeFrameBuffer& operator=(const VkeFrameBuffer&&) = delete;

		uint32_t width, height;
		VkFramebuffer framebuffer;
		VkRenderPass renderPass;
		VkSampler sampler;
		std::vector<FrameBufferAttachment> attachments;

		uint32_t addAttachment(AttachmentCreateInfo createinfo);
		VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode);
		VkResult createRenderPass();
	private:
		VkeDevice& m_device;
	};
}