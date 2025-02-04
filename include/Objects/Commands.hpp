#pragma once

#include <vulkan/vulkan.h>

namespace gr {

	class Semaphore;
	class Fence;

	class CommandPool {
	public:
		CommandPool(uint32_t index) { init(index); }
		CommandPool(void) {}
		~CommandPool(void);

		void init(uint32_t index);

		VkCommandPool ptr = nullptr;
	private:
	};

	class CommandBuffer {
	public:
		CommandBuffer(CommandPool& a) { init(a); }
		CommandBuffer(void) {};
		~CommandBuffer(void);
		void init(CommandPool&);
		void begin(bool _reset = true);
		void end(void);
		void submit(VkSemaphore *wait, VkSemaphore *signal, VkFence *in_flight);
		inline void reset(void) { vkResetCommandBuffer(ptr, 0); }


		VkCommandBuffer ptr = nullptr;
		VkCommandPool pool = nullptr;
	private:
	};
}