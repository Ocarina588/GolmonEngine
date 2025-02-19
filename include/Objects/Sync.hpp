#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include <array>

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

	template<int N = 2>
	class PolySemaphore {
	public:
		PolySemaphore(void) = default;
		~PolySemaphore(void) = default;

		operator VkSemaphore(void) { return semaphores[index]; } 
		operator VkSemaphore* (void) { return semaphores[index]; }
		operator Semaphore&(void) { return semaphores[index]; }
		operator Semaphore* (void) { return semaphores[index]; }
		
		inline void init(void) { for (auto& i : semaphores) i.init(); }
		inline void next(void) { ++index = index % N; }

	private:
		std::array<Semaphore, N> semaphores;
		uint32_t index = 0;
	};

	template<int N = 2>
	class PolyFence {
	public:
		PolyFence(void) = default;
		~PolyFence(void) = default;

		operator VkFence(void) { return fences[index]; }
		operator VkFence* (void) { return fences[index]; }
		operator Fence&(void) { return fences[index]; }
		operator Fence* (void) { return fences[index]; }

		inline void next(void) { ++index = index % N; }
		inline void init(bool signaled = true) { for (auto& i : fences) i.init(signaled); }
		inline void wait(bool _reset = true) { if (vkWaitForFences(ge::ctx::device.ptr, 1, fences[index], VK_TRUE, UINT64_MAX) != VK_SUCCESS) throw std::runtime_error("failed to wait for fences"); if (_reset) fences[index].reset(); }
		inline void reset(void) { vkResetFences(ge::ctx::device.ptr, 1, fences[index]); }

	private:
		std::array<Fence, N> fences;
		uint32_t index = 0;
	};
}