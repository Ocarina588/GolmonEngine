#pragma once

#include <vulkan/vulkan.h>
#include <vector>

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

		uint32_t render_pass_item = 0;
		uint32_t light_equation_type = 0;

	};
}