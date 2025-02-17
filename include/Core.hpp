#pragma once

#include "GolmonEngine.hpp"
#include "UI/UI.hpp"

class Core {
public:

	Core(void);
	~Core(void);

	int main(int ac, char** av);
	void updates(void);

	ge::Events events;
	ge::Camera camera;
	ge::Mesh mesh;
	ge::Sampler sampler;

	ge::Image depth_image, background;
	ge::Semaphore image_acquired, finished_rendering;
	ge::Fence in_flight;
	ge::CommandPool command_pool;
	ge::CommandBuffer command_buffer;
	ge::RenderPass render_pass;
	ge::DescriptorPool descriptors;
	ge::GraphicsPipeline gp;
	ge::UI ui;

};