#include "GolmonRenderer.hpp"

using namespace gr;

void CommandPool::init(uint32_t index)
{
	VkCommandPoolCreateInfo create_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	create_info.queueFamilyIndex = index;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("failed to create command pool");
}

CommandPool::~CommandPool(void)
{
	vkDestroyCommandPool(ctx::device.ptr, ptr, nullptr);
}

void CommandBuffer::init(CommandPool& command_pool)
{
	VkCommandBufferAllocateInfo create_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	create_info.commandPool = command_pool.ptr;
	create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	create_info.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(ctx::device.ptr, &create_info, &ptr) != VK_SUCCESS) 
		throw std::runtime_error("failed to alloc command buffers");

	pool = command_pool.ptr;
}

CommandBuffer::~CommandBuffer(void)
{
	vkFreeCommandBuffers(ctx::device.ptr, pool, 1, &ptr);
}

void CommandBuffer::begin(bool _reset)
{
	if (_reset) reset();
	VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(ptr, &begin_info) != VK_SUCCESS) 
		throw std::runtime_error("failed to begin command buffer");
}

void CommandBuffer::end(void)
{
	if (vkEndCommandBuffer(ptr) != VK_SUCCESS) throw std::runtime_error("failed to end command buffer");
}

void CommandBuffer::submit(VkSemaphore *wait, VkSemaphore *signal, VkFence *in_flight)
{
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &ptr;
	submit_info.pSignalSemaphores = signal;
	submit_info.pWaitSemaphores = wait;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.signalSemaphoreCount = signal != nullptr;
	submit_info.waitSemaphoreCount = wait != nullptr;

	VkFence f = in_flight ? *in_flight : nullptr;

	if (vkQueueSubmit(ctx::device.queue.graphics, 1, &submit_info, f) != VK_SUCCESS)
		throw std::runtime_error("failed to submit");
}