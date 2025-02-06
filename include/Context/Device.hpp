#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace ge {
	
	class Device {
		friend class ctx;
		friend class Window;
		friend class Instance;

	public:
		Device(void);
		~Device(void);

		void init(void);

		operator VkDevice() { return ptr; };
		operator VkPhysicalDevice() { return physical_ptr; };

		VkDevice ptr = nullptr;
		VkPhysicalDevice physical_ptr = nullptr;

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memory_properties;

		std::vector<VkQueueFamilyProperties> family_properties;

		VkSurfaceFormatKHR format = {
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};

		VkExtent2D extent = {};

		struct {
			VkQueue graphics = nullptr;
			VkQueue compute = nullptr;
			VkQueue transfer = nullptr;
			VkQueue present = nullptr;
		} queue;

		struct {
			uint32_t graphics = ~(0u);
			uint32_t compute = ~(0u);
			uint32_t transfer = ~(0u);
			uint32_t present = ~(0u);
		} index;

	private:

		void dump(void);
		void get_info(void);
		std::vector<VkDeviceQueueCreateInfo> choose_queues(void);

		std::vector<char const*> _extensions;
		uint32_t _gpu = 0;
		float priority = 1.f;

	};

	VkMemoryRequirements get_memory_requirements(VkImage image);
	VkMemoryRequirements get_memory_requirements(VkBuffer buffer);
	uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
	void wait_idle(void);
}