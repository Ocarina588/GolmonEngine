#include "GolmonRenderer.hpp"
#include "Context/Context.hpp"

gr::Instance gr::ctx::instance;
gr::Device gr::ctx::device;
gr::Window gr::ctx::window;

using Vk = gr::ctx;

void gr::ctx::use_debug(void)
{
#ifdef _DEBUG
	Vk::instance._debug = true;
#endif
}

void gr::ctx::set_extent(uint32_t w, uint32_t h)
{
	Vk::device.extent = { w, h };
}

void gr::ctx::use_window(char const* t)
{
	Vk::window.init(static_cast<int>(Vk::device.extent.width), static_cast<int>(Vk::device.extent.height), t);
}

void gr::ctx::add_layer(char const* l)
{
	Vk::instance._layers.push_back(l);
}

void gr::ctx::add_instance_extension(char const* e)
{
	Vk::instance._extensions.push_back(e);
}

void gr::ctx::add_device_extension(char const* e)
{
	Vk::device._extensions.push_back(e);
}

void gr::ctx::use_gpu(uint32_t index)
{
	Vk::device._gpu = index;
}

void gr::ctx::init(void)
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