#pragma once

#include <vulkan/vulkan.h>

namespace gr {

	class Image {
	public:
		Image(void);
		~Image(void);

		void init(void);

		static VkImage create_image(void);
		static VkImageView create_view(VkImage image, VkFormat format, VkImageAspectFlags aspect);
		static VkDeviceMemory create_memory(void);

	private:
		VkImage ptr = nullptr;
		VkImageView view = nullptr;
		VkDeviceMemory memory = nullptr;
	};

}