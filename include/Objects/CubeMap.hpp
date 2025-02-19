#pragma once

#include <vulkan/vulkan.h>
#include "Objects/GraphicsPipeline.hpp"

namespace ge {

	class CubeMap {
	public:
		CubeMap(void);
		~CubeMap(void);

		void init(char const* file_name);
	private:
		VkImage image = nullptr;
		VkImageView view[6] = { nullptr };
		VkFramebuffer fb[6] = { nullptr };
		VkDeviceMemory memory = nullptr;
		ge::RenderPass render_pass;
	};
}