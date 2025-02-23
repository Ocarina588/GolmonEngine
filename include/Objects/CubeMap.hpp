#pragma once

#include <vulkan/vulkan.h>
#include "Objects/GraphicsPipeline.hpp"
#include "Assets/Assets.hpp"

namespace ge {

	class CubeMap {
	public:
		CubeMap(void);
		~CubeMap(void);

		void init(char const* file_name, ge::CommandBuffer &co, ge::RenderPass &render_pass);
		void init_vertices(ge::CommandBuffer& co);
		inline void draw(ge::CommandBuffer& co) { mesh.draw(co); }
	private:
		VkImage image = nullptr;
		VkImageView view[6] = { nullptr };
		VkFramebuffer fb[6] = { nullptr };
		VkDeviceMemory memory = nullptr;
		ge::Mesh mesh;
		ge::GraphicsPipeline gp;
	};
}