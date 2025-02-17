#pragma once

#include <vulkan/vulkan.h>

class Core;

namespace ge {


	class UI {
	public:
		UI();
		~UI();
		void init(Core *core);
		void render(VkCommandBuffer cmd);
		VkDescriptorPool imguiPool = nullptr;
		Core* core = nullptr;
	};
}