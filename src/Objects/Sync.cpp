#include "GolmonEngine.hpp"

ge::Semaphore::Semaphore(void)
{
	if (ge::ctx::device.ptr)
		init();
}

ge::Semaphore::~Semaphore(void)
{
	vkDestroySemaphore(ctx::device.ptr, ptr, nullptr);
}

void ge::Semaphore::init(void)
{
	if (ptr) return;
	VkSemaphoreCreateInfo create_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("faield to create semaphore");
}

ge::Fence::Fence(bool signaled)
{
	if (ge::ctx::device.ptr)
		init(signaled);
}

ge::Fence::~Fence(void)
{
	vkDestroyFence(ctx::device.ptr, ptr, nullptr);
}

void ge::Fence::init(bool signaled)
{
	if (ptr) return;
	VkFenceCreateInfo create_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	if (vkCreateFence(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("failed to create fence");
}