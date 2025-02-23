#pragma once

#include <array>
#include "GolmonEngine.hpp"
#include "UI/UI.hpp"
#define IN_FLIGHT_NUMBER 3
class Core {
public:

	Core(void);
	~Core(void);

	void init_cube_map(void);

	int main(int ac, char** av);
	void updates(void);

	ge::Events events;
	ge::Camera camera;
	ge::Sampler sampler;
	ge::Mesh cube_map;
	ge::Image depth_image;
	ge::CommandPool command_pool;

	ge::PolyCommandBuffer<IN_FLIGHT_NUMBER> command_buffer;
	ge::PolySemaphore<IN_FLIGHT_NUMBER> image_acquired, finished_rendering;
	ge::PolyFence<IN_FLIGHT_NUMBER> in_flight;

	ge::RenderPass render_pass;
	ge::DescriptorPool descriptors;
	ge::GraphicsPipeline gp, gb;
	ge::UI ui;

};