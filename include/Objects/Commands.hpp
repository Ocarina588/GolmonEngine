#pragma once

#include <vulkan/vulkan.h>
#include <array>

namespace ge {

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

		operator VkCommandBuffer (void) { return ptr; }
		operator VkCommandBuffer* (void) { return &ptr; }

		void init(CommandPool&);
		void begin(bool _reset = true);
		void end(void);
		void submit(VkSemaphore *wait, VkSemaphore *signal, VkFence *in_flight);
		inline void reset(void) { vkResetCommandBuffer(ptr, 0); }

		VkCommandBuffer ptr = nullptr;
		VkCommandPool pool = nullptr;
	private:
	};

	template<int N = 2>
	class PolyCommandBuffer {
	public:
		PolyCommandBuffer(void) = default;
		~PolyCommandBuffer(void) = default;

		operator VkCommandBuffer (void) { return command_buffers[index]; }
		operator VkCommandBuffer* (void) { return command_buffers[index]; }
		operator CommandBuffer& (void) { return command_buffers[index]; }
		operator CommandBuffer* (void) { return command_buffers[index]; }
		inline void init(CommandPool& pool) { for (auto& i : command_buffers) i.init(pool); }
		inline void next(void) { ++index = index % N; }
		inline void begin(bool reset = true) { command_buffers[index].begin(reset); }
		inline void end(void) { command_buffers[index].end(); }
		inline void submit(VkSemaphore* wait, VkSemaphore* signal, VkFence* in_flight) {
			command_buffers[index].submit(wait, signal, in_flight);
		}
		inline void reset(void) { command_buffers[index].reset(); }

	private:
		std::array<CommandBuffer, N> command_buffers;
		uint32_t index = 0;
	};
}