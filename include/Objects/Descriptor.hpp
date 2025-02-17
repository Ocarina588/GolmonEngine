#pragma once

#include <iostream>
#include <vulkan/vulkan.h>

namespace ge {

	class DescriptorPool {
	public:
		DescriptorPool(void);
		~DescriptorPool(void);

		void init(void);
		inline DescriptorPool& add_set(uint32_t size) { bindings.push_back({}); sizes.push_back(size); return *this; }
		inline DescriptorPool& add_binding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, uint32_t count = 1) {
			bindings.rbegin()->emplace_back(binding, type, count, stage, nullptr);
			return *this;
		}
		inline VkDescriptorSet& get_set(uint32_t set, uint32_t index) {
			uint32_t offset = 0;
			for (uint32_t i = 0; i < set; i++)
				offset += sizes[i];
			return sets[offset + index];
		}
		void add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkBuffer buffer);
		void add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkSampler sampler, VkImageView view, VkImageLayout layout);
		void add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL, VkSampler sampler = nullptr);
		void add_writes(uint32_t set, uint32_t index, uint32_t binding_index, std::vector<ge::Image> const& infos, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
		void add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkAccelerationStructureKHR& as);


		void write(void);

		std::vector<uint32_t> sizes;
		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkDescriptorSet> sets;
		std::vector<VkWriteDescriptorSet> writes;

		VkBaseOutStructure* test;
		VkDescriptorPool ptr = nullptr;

		std::vector<VkDescriptorBufferInfo> buffers_info;
		std::vector<VkDescriptorImageInfo> images_info;
		std::vector<VkWriteDescriptorSetAccelerationStructureKHR> tlases_info;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
	};


}