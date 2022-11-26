#pragma once

#include "vke_device.hpp"
#include "vke_buffer.hpp"
#include "vke_descriptors.hpp"
#include "vke_frame_info.hpp"

#include <array>
#include <memory>
#include <vector>

#define NUM_DESCRIPTOR_SETS 2
#define MAX_POOL_SIZE 1000

// Note from past experiments, this class CANNOT be static. Can't call destructors on static objects.
// I want to overhaul descriptors entirely. The current usage scales poorly, and cannot easily add incoming data to shaders
// Check out these resources:
// https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers
// https://github.com/KhronosGroup/Vulkan-Samples/tree/master/samples/extensions/descriptor_indexing
namespace vke {
	class VkeCore {
	public:
		std::unique_ptr<VkeDescriptorPool> globalDescriptorPool;
		std::unique_ptr<VkeDescriptorSetLayout> globalSetLayout;

		std::unique_ptr<VkeDescriptorPool> shadowDescriptorPool;
		std::unique_ptr<VkeDescriptorSetLayout> shadowSetLayout;

		// Individual sets
		std::vector<VkDescriptorSet> objectSet;
		std::vector<VkDescriptorSet> shadowSet;

		// Individual bindings within sets
		// set = 0, binding = 0, 1, etc.
		std::vector<std::unique_ptr<VkeBuffer>> objectBuffers;
		std::vector<std::unique_ptr<VkeBuffer>> sceneBuffers;
		
		// Array of set buffer vectors
		std::vector<VkDescriptorSet> descriptorSets[NUM_DESCRIPTOR_SETS];
		std::vector<VkDescriptorSet> getSets(uint32_t frameIndex);

		void init(VkeDevice& device);
		void buildCoreDescriptorSets();
		std::vector<VkDescriptorSetLayout> getSetLayouts();

		~VkeCore();
	};
}