#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace gr {
	
	class Device {
		friend class Context;
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
		std::vector<VkQueueFamilyProperties> family_properties;

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

		//VkSurfaceFormatKHR format = {
		//	.format = VK_FORMAT_R8G8B8_SRGB,
		//	.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		//};

		void dump(void);
		void get_info(void);
		std::vector<VkDeviceQueueCreateInfo> choose_queues(void);

		std::vector<char const*> _extensions;
		uint32_t _gpu = 0;
	};
}