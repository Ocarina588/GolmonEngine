#pragma once

#include <vulkan/vulkan.h>

namespace ge {

	class UI {
	public:
		UI();
		~UI();
		void init(VkRenderPass render_pass);
		void render(VkCommandBuffer cmd);
		VkDescriptorPool imguiPool = nullptr;
	};
}