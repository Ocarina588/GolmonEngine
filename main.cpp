#include <iostream>
#include "GolmonRenderer.hpp"

using Vk = gr::Context;

int main(int ac, char** av)
{
	try {
		Vk::add_layer("VK_LAYER_LUNARG_monitor");
		Vk::use_debug();
		Vk::use_window(1280, 720, "Vulkan App"); 
		Vk::use_gpu(0);

		Vk::init();

		while (Vk::window.is_open())
			Vk::window.poll_events();
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}