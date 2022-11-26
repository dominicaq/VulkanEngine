#pragma once

#include "vke_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vke {
    class VkeDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VkeDevice& device) : m_device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VkeDescriptorSetLayout> build() const;
            std::shared_ptr<VkeDescriptorSetLayout> buildShared() const;
        private:
            VkeDevice& m_device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VkeDescriptorSetLayout(VkeDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VkeDescriptorSetLayout();
        VkeDescriptorSetLayout(const VkeDescriptorSetLayout&) = delete;
        VkeDescriptorSetLayout& operator=(const VkeDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        VkeDevice& m_device;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

        friend class VkeDescriptorWriter;
    };

    class VkeDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VkeDevice& device) : m_device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VkeDescriptorPool> build() const;
            std::shared_ptr<VkeDescriptorPool> buildShared() const;
        private:
            VkeDevice& m_device;
            std::vector<VkDescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets = 1000;
            VkDescriptorPoolCreateFlags m_poolFlags = 0;
        };

        VkeDescriptorPool(
            VkeDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VkeDescriptorPool();
        VkeDescriptorPool(const VkeDescriptorPool&) = delete;
        VkeDescriptorPool& operator=(const VkeDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VkeDevice& m_device;
        VkDescriptorPool m_descriptorPool;

        friend class VkeDescriptorWriter;
    };

    class VkeDescriptorWriter {
    public:
        VkeDescriptorWriter(VkeDescriptorSetLayout& setLayout, VkeDescriptorPool& pool);

        VkeDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VkeDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VkeDescriptorSetLayout& m_setLayout;
        VkeDescriptorPool& m_pool;
        std::vector<VkWriteDescriptorSet> m_writes;
    };
}