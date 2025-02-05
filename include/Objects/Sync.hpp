#pragma once
#include <iostream>
#include <vulkan/vulkan.h>

#include "Context/Context.hpp"
#include "Context/Device.hpp"

namespace ge {

	class Semaphore {
	public:
		Semaphore(void);
		~Semaphore(void);

		operator VkSemaphore() { return ptr; };
		operator VkSemaphore* () { return &ptr; };

		void init(void);

		VkSemaphore ptr = nullptr;
	private:
	};

	class Fence {
	public:
		Fence(bool signaled = true);
		~Fence(void);

		operator VkFence () { return ptr; };
		operator VkFence* () { return &ptr; };

		void init(bool signaled = true);
		inline void wait(bool _reset = true) { if (vkWaitForFences(ge::ctx::device.ptr, 1, &ptr, VK_TRUE, UINT64_MAX) != VK_SUCCESS) throw std::runtime_error("failed to wait for fences"); if (_reset) reset(); }
		inline void reset(void) { vkResetFences(ge::ctx::device.ptr, 1, &ptr); }

		VkFence ptr = nullptr;
	private:
	};
}