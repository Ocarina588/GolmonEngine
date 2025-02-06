#include "GolmonEngine.hpp"
#include "Context/Context.hpp"

ge::Instance ge::ctx::instance;
ge::Device ge::ctx::device;
ge::Window ge::ctx::window;

using Vk = ge::ctx;

void ge::ctx::use_debug(void)
{
#ifdef _DEBUG
	Vk::instance._debug = true;
#endif
}

void ge::ctx::set_extent(uint32_t w, uint32_t h)
{
	Vk::device.extent = { w, h };
}

void ge::ctx::set_mode(VkPresentModeKHR mode)
{
	Vk::window.mode = mode;
}

void ge::ctx::use_window(char const* t)
{
	Vk::window.init(static_cast<int>(Vk::device.extent.width), static_cast<int>(Vk::device.extent.height), t);
}

void ge::ctx::add_layer(char const* l)
{
	Vk::instance._layers.push_back(l);
}

void ge::ctx::add_instance_extension(char const* e)
{
	Vk::instance._extensions.push_back(e);
}

void ge::ctx::add_device_extension(char const* e)
{
	Vk::device._extensions.push_back(e);
}

void ge::ctx::use_gpu(uint32_t index)
{
	Vk::device._gpu = index;
}

void ge::ctx::init(void)
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