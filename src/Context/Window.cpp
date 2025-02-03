#include <iostream>
#include <algorithm>

#include "GolmonRenderer.hpp"
#include "utils.hpp"

using Vk = gr::Context;

gr::Window::Window(void)
{

}

gr::Window::~Window(void)
{
	if (ptr == nullptr) return;

	vkDestroySwapchainKHR(Vk::device, Vk::window.swapchain, nullptr);
	vkDestroySurfaceKHR(Vk::instance, surface, nullptr);
	glfwDestroyWindow(ptr);
	glfwTerminate();
}

void gr::Window::init(int w, int h, char const* t)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!(ptr = glfwCreateWindow(w, h, t, nullptr, nullptr)))
		throw std::runtime_error("failed to create window");

	title = t;

	dump();
}

std::vector<char const *> gr::Window::get_required_extensions(void)
{
	uint32_t count = 0;
	char const ** tmp = glfwGetRequiredInstanceExtensions(&count);

	return std::vector<char const*>(tmp, tmp + count);
}

void gr::Window::init_surface(void)
{
	if (glfwCreateWindowSurface(Vk::instance, ptr, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create surface");
}

VkExtent2D gr::Window::get_extent(void)
{
	int w = 0;
	int h = 0;
	glfwGetWindowSize(ptr, &w, &h);

	return {
		static_cast<uint32_t>(w),
		static_cast<uint32_t>(h)
	};
}

void gr::Window::init_swapchain(void)
{

	VkSwapchainCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

	create_info.surface = Vk::window.surface;
	create_info.minImageCount = Vk::window.capabilites.minImageCount + 1;
	if (Vk::window.capabilites.maxImageCount && create_info.minImageCount > Vk::window.capabilites.maxImageCount)
		create_info.minImageCount = Vk::window.capabilites.maxImageCount;
	create_info.imageFormat = formats[0].format;
	create_info.imageColorSpace = formats[0].colorSpace;
	create_info.imageExtent = get_extent();
	create_info.imageExtent.width = std::clamp(create_info.imageExtent.width, 
		Vk::window.capabilites.minImageExtent.width, Vk::window.capabilites.maxImageExtent.width);
	create_info.imageExtent.height = std::clamp(create_info.imageExtent.height,
		Vk::window.capabilites.minImageExtent.height, Vk::window.capabilites.maxImageExtent.height);
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (Vk::device.index.present != Vk::device.index.graphics)
		throw std::runtime_error("need to setup sharing mode concurrent");
	create_info.preTransform = Vk::window.capabilites.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = Vk::window.mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = nullptr;

	if (vkCreateSwapchainKHR(Vk::device, &create_info, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swapchain");

	init_swapchain_resources();

	dump_swapchain();
}

void gr::Window::init_swapchain_resources(void)
{
	uint32_t count = 0;
	vkGetSwapchainImagesKHR(Vk::device, swapchain, &count, nullptr);
	images.resize(count);
	views.resize(count);
	framebuffers.resize(count);
	vkGetSwapchainImagesKHR(Vk::device, swapchain, &count, images.data());

	for (uint32_t i = 0; i < count; i++) {

	}
}

void gr::Window::dump(void)
{
	std::cout << TERMINAL_COLOR_CYAN << "Window creation SUCCESS" << std::endl;
	std::cout << TERMINAL_COLOR_RESET;
}

void gr::Window::dump_swapchain(void)
{
	char const* present_modes[] = {
		"VK_PRESENT_MODE_IMMEDIATE_KHR",
		"VK_PRESENT_MODE_MAILBOX_KHR",
		"VK_PRESENT_MODE_FIFO_KHR",
		"VK_PRESENT_MODE_FIFO_RELAXED_KHR"
	};

	std::cout << TERMINAL_COLOR_YELLOW << "Swapchain capabilites:" << std::endl;

	if (Vk::window.mode > VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		throw std::runtime_error("present mode dump out off bound");

	std::cout << TAB << TERMINAL_COLOR_GREEN << "Present mode: ";
	std::cout << TERMINAL_COLOR_MAGENTA << std::string(present_modes[Vk::window.mode]) << std::endl;

	std::cout << TAB << TERMINAL_COLOR_GREEN << "Images: ";
	std::cout << TERMINAL_COLOR_MAGENTA << std::to_string(images.size()) << std::endl;

	std::cout << TERMINAL_COLOR_CYAN << "Swapchain creation SUCCESS" << std::endl;
	std::cout << TERMINAL_COLOR_RESET;
}