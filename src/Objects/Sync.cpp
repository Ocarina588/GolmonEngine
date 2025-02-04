#include "GolmonRenderer.hpp"

gr::Semaphore::Semaphore(void)
{
	if (gr::ctx::device.ptr)
		init();
}

gr::Semaphore::~Semaphore(void)
{
	vkDestroySemaphore(ctx::device.ptr, ptr, nullptr);
}

void gr::Semaphore::init(void)
{
	VkSemaphoreCreateInfo create_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("faield to create semaphore");
}

gr::Fence::Fence(bool signaled)
{
	if (gr::ctx::device.ptr)
		init(signaled);
}

gr::Fence::~Fence(void)
{
	vkDestroyFence(ctx::device.ptr, ptr, nullptr);
}

void gr::Fence::init(bool signaled)
{
	VkFenceCreateInfo create_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	if (vkCreateFence(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("failed to create fence");

}