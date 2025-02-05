#include "GolmonEngine.hpp"

using namespace ge;

Buffer::Buffer(uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	init(_size, usage, properties);
}

void Buffer::init(uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkMemoryAllocateFlags flag)
{
	original_size = _size;
	VkBufferCreateInfo create_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	create_info.size = _size;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.usage = usage;

	if (vkCreateBuffer(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	auto requirements = ge::get_memory_requirements(ptr);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
	memoryAllocateFlagsInfo.flags = flag;

	VkMemoryAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	alloc_info.allocationSize = requirements.size;
	alloc_info.pNext = &memoryAllocateFlagsInfo;
	alloc_info.memoryTypeIndex = ge::find_memory_type(requirements.memoryTypeBits, properties);

	if (vkAllocateMemory(ctx::device.ptr, &alloc_info, nullptr, &memory) != VK_SUCCESS)
		throw std::runtime_error("failed to alloc memory");

	size = requirements.size;
	std::cout << size << std::endl;
	vkBindBufferMemory(ctx::device.ptr, ptr, memory, 0);

	VkBufferDeviceAddressInfo address_info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	address_info.buffer = ptr;

	//if (flag & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)
	//	address = RayTracingPipeline::vkGetBufferDeviceAddressKHR(ctx::device.ptr, &address_info);
}

Buffer::~Buffer(void)
{
	vkFreeMemory(ctx::device.ptr, memory, nullptr);
	vkDestroyBuffer(ctx::device.ptr, ptr, nullptr);
}