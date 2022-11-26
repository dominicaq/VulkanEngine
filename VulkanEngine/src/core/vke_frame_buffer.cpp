#include "vke_frame_buffer.hpp"

namespace vke {
	VkeFrameBuffer::VkeFrameBuffer(VkeDevice& device) : m_device{ device } {}

	VkeFrameBuffer::~VkeFrameBuffer() {
		vkDestroySampler(m_device.device(), sampler, nullptr);

		for (int i = 0; i < attachments.size(); i++) {
			vkDestroyImageView(m_device.device(), attachments[i].view, nullptr);
			vkDestroyImage(m_device.device(), attachments[i].image, nullptr);
			vkFreeMemory(m_device.device(), attachments[i].memory, nullptr);
		}

		vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
		vkDestroyRenderPass(m_device.device(), renderPass, nullptr);
	}

	uint32_t VkeFrameBuffer::addAttachment(AttachmentCreateInfo createinfo) {
		FrameBufferAttachment attachment;
		attachment.format = createinfo.format;

		VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

		// Select aspect mask and layout depending on usage

		// Color attachment
		if (createinfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		// Depth (and/or stencil) attachment
		if (createinfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			if (attachment.hasDepth())
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			if (attachment.hasStencil())
			{
				aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		assert(aspectMask > 0);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = createinfo.format;
		imageInfo.extent.width = createinfo.width;
		imageInfo.extent.height = createinfo.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = createinfo.layerCount;
		imageInfo.samples = createinfo.imageSampleCount;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = createinfo.usage;

		m_device.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			attachment.image, 
			attachment.memory);

		attachment.subReourceRange = {};
		attachment.subReourceRange.aspectMask = aspectMask;
		attachment.subReourceRange.levelCount = 1;
		attachment.subReourceRange.layerCount = createinfo.layerCount;

		VkImageViewCreateInfo imageView{};
		imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView.viewType = (createinfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		imageView.format = createinfo.format;
		imageView.subresourceRange = attachment.subReourceRange;
		imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
		imageView.image = attachment.image;
		if (vkCreateImageView(m_device.device(), &imageView, nullptr, &attachment.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer image view!");
		}

		// Fill attachment description
		attachment.description = {};
		attachment.description.samples = createinfo.imageSampleCount;
		attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.description.storeOp = (createinfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.description.format = createinfo.format;
		attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Final layout
		// If not, final layout depends on attachment type
		if (attachment.hasDepth() || attachment.hasStencil())
		{
			attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		attachments.push_back(attachment);

		return static_cast<uint32_t>(attachments.size() - 1);
	}

	VkResult VkeFrameBuffer::createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		return vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &sampler);
	}

	VkResult VkeFrameBuffer::createRenderPass() {
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		for (auto& attachment : attachments)
		{
			attachmentDescriptions.push_back(attachment.description);
		};

		// Collect attachment references
		std::vector<VkAttachmentReference> colorReferences;
		VkAttachmentReference depthReference = {};
		bool hasDepth = false;
		bool hasColor = false;

		uint32_t attachmentIndex = 0;

		for (auto& attachment : attachments)
		{
			if (attachment.isDepthStencil())
			{
				// Only one depth attachment allowed
				assert(!hasDepth);
				depthReference.attachment = attachmentIndex;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				hasDepth = true;
			}
			else
			{
				colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
				hasColor = true;
			}
			attachmentIndex++;
		};

		// Default render pass setup uses only one subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		if (hasColor)
		{
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		}
		if (hasDepth)
		{
			subpass.pDepthStencilAttachment = &depthReference;
		}

		// Use subpass dependencies for attachment layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create render pass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();
		if (vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer render pass!");
		}

		std::vector<VkImageView> attachmentViews;
		for (auto attachment : attachments)
		{
			attachmentViews.push_back(attachment.view);
		}

		// Find. max number of layers across attachments
		uint32_t maxLayers = 0;
		for (auto attachment : attachments)
		{
			if (attachment.subReourceRange.layerCount > maxLayers)
			{
				maxLayers = attachment.subReourceRange.layerCount;
			}
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.pAttachments = attachmentViews.data();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = maxLayers;
		if (vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer!");
		}
		
		return VK_SUCCESS;
	}
}