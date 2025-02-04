#pragma once
#include "Context/Context.hpp"

namespace gr {

	class Buffer {
	public:
		Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		Buffer(void* p, uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) { init(p, _size, usage, properties); }
		Buffer(void) {};
		~Buffer(void);

		void init(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags flag = 0);
		inline void init(void* p, uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags flag = 0) {
			init(_size, usage, properties, flag);
			map();
			memcpy(p, _size);
			unmap();
		}

		inline void map(void) { if (vkMapMemory(ctx::device.ptr, memory, 0, size, 0, &data) != VK_SUCCESS) throw std::runtime_error("failed to map memory"); }
		inline void unmap(void) { vkUnmapMemory(ctx::device.ptr, memory); }
		inline void memcpy(void* p, uint32_t _size) { std::memcpy(data, p, _size); }

		void* data = nullptr;

		VkBuffer ptr = nullptr;
		VkDeviceMemory memory = nullptr;
		VkDeviceAddress address = 0;
		VkDeviceSize size = 0;
		VkDeviceSize original_size = 0;
	private:
	};

}