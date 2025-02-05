#pragma once

#include "GolmonEngine.hpp"

struct Vertex {
	glm::vec3 pos;
	static VkVertexInputBindingDescription get_binding(void)
	{
		VkVertexInputBindingDescription binding{};

		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex);

		return binding;
	}
	static VkVertexInputAttributeDescription get_attribute(void)
	{
		VkVertexInputAttributeDescription attribute{};
		attribute.binding = 0;
		attribute.offset = offsetof(Vertex, pos);
		attribute.format = VK_FORMAT_R32G32B32_SFLOAT;

		return attribute;
	}
};

class Core {
public:

	Core(void);
	~Core(void);

	int main(int ac, char** av);
	void updates(void);

	ge::Events events;
	ge::Camera camera;
	ge::Mesh mesh;

	ge::Image depth_image;
	ge::Semaphore image_acquired, finished_rendering;
	ge::Fence in_flight;
	ge::CommandPool command_pool;
	ge::CommandBuffer command_buffer;
	ge::RenderPass render_pass;
	ge::DescriptorPool descriptors;
	ge::GraphicsPipeline gp;
};