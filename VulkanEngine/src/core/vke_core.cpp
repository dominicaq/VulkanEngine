#pragma once

#include "vke_core.hpp"

#define MAX_FRAMES_IN_FLIGHT 2
// https://github.com/lukasino1214/StellarEngine/blob/main/Engine/graphics/core.h
// https://github.com/lukasino1214/StellarEngine/blob/main/Engine/graphics/core.cpp
namespace vke {
    void VkeCore::init(VkeDevice& device) {
        globalDescriptorPool = VkeDescriptorPool::Builder(device)
            .setMaxSets(MAX_POOL_SIZE)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000) // Object
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000) // Lighting
            .build();

        globalSetLayout = VkeDescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS) // Object
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS) // Lighting
            .build();

        shadowDescriptorPool = VkeDescriptorPool::Builder(device)
            .setMaxSets(MAX_POOL_SIZE)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000) // Shadow map
            .build();

        shadowSetLayout = VkeDescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Shadow map
            .build();

        // Init sets
        uint32_t size = MAX_FRAMES_IN_FLIGHT;
        objectSet = std::vector<VkDescriptorSet>(size);
        shadowSet = std::vector<VkDescriptorSet>(size);

        // Init uniform buffer objects
        objectBuffers = std::vector<std::unique_ptr<VkeBuffer>>(size);
        sceneBuffers = std::vector<std::unique_ptr<VkeBuffer>>(size);
        for (int i = 0; i < size; i++) {
            objectBuffers[i] = std::make_unique<VkeBuffer>(
                device,
                sizeof(UniformBufferObject),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            objectBuffers[i]->map();

            sceneBuffers[i] = std::make_unique<VkeBuffer>(
                device,
                sizeof(UniformBufferScene),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            sceneBuffers[i]->map();
        }
    }

    std::vector<VkDescriptorSetLayout> VkeCore::getSetLayouts() {
        // In order of set #
        std::vector<VkDescriptorSetLayout> setLayouts{};
        setLayouts.push_back(globalSetLayout->getDescriptorSetLayout());
        setLayouts.push_back(shadowSetLayout->getDescriptorSetLayout());
        return setLayouts;
    }

    void VkeCore::buildCoreDescriptorSets() {
        uint32_t size = MAX_FRAMES_IN_FLIGHT;
        for (int i = 0; i < (int)size; i++) {
            auto objectBuffer = objectBuffers[i]->descriptorInfo();
            auto sceneBuffer = sceneBuffers[i]->descriptorInfo();

            VkeDescriptorWriter(*globalSetLayout, *globalDescriptorPool)
                .writeBuffer(0, &objectBuffer)
                .writeBuffer(1, &sceneBuffer)
                .build(objectSet[i]);
        }

        descriptorSets[0] = objectSet;
    }

    std::vector<VkDescriptorSet> VkeCore::getSets(uint32_t frameIndex) {
        std::vector<VkDescriptorSet> sets;
        for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++) {
            sets.push_back(descriptorSets[i][frameIndex]);
        }
        return sets;
    }

    VkeCore::~VkeCore() {};
}