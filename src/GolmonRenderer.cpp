#include "GolmonRenderer.hpp"

gr::Instance gr::Context::instance;
gr::Device gr::Context::device;
gr::Window gr::Context::window;

using Vk = gr::Context;

void gr::Context::use_debug(void)
{
	Vk::instance._debug = true;
}

void gr::Context::use_window(int w, int h, char const* t)
{
	Vk::window.init(w, h, t);
}

void gr::Context::add_layer(char const* l)
{
	Vk::instance._layers.push_back(l);
}

void gr::Context::add_instance_extension(char const* e)
{
	Vk::instance._extensions.push_back(e);
}

void gr::Context::add_device_extension(char const* e)
{
	Vk::device._extensions.push_back(e);
}

void gr::Context::use_gpu(uint32_t index)
{
	Vk::device._gpu = index;
}

void gr::Context::init(void)
{
	if (window.ptr) {
		for (auto e : window.get_required_extensions())
			instance._extensions.push_back(e);
		device._extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	}

	if (instance._debug) {
		instance._extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance._layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	instance.init();

	if (window.ptr)
		window.init_surface();

	device.init();

	if (window.ptr)
		window.init_swapchain();
}