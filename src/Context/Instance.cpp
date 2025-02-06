#include <iostream>
#include "GolmonEngine.hpp"

using Vk = ge::ctx;

ge::Instance::Instance(void)
{
	app_info.pEngineName = "GolmonRenderer";
	app_info.pApplicationName = "VulkanApp";
	app_info.apiVersion = VK_MAKE_VERSION(1, 3, 0);
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
}

ge::Instance::~Instance(void)
{
	if (_debug_messenger)
		destroy_debug_messenger();
	vkDestroyInstance(ptr, nullptr);
}

VkBool32 VKAPI_PTR debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cout << TERMINAL_COLOR_MAGENTA << "Validation Layer:" << std::endl;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		std::cout << TERMINAL_COLOR_YELLOW;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		std::cout << TERMINAL_COLOR_RED;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		break;
	default:
		break;
	}
	std::cout << pCallbackData->pMessage << std::endl;
	std::cout << TERMINAL_COLOR_RESET;

	return VK_SUCCESS;
}

static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info(void)
{
	VkDebugUtilsMessengerCreateInfoEXT create_info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;// | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	create_info.messageType = 0b1111;
	create_info.pfnUserCallback = debug_callback;

	return create_info;
}

void ge::Instance::create_debug_messenger(void)
{
	VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_messenger_create_info();
	auto func = GET_INSTANCE_PROC(ptr, PFN_vkCreateDebugUtilsMessengerEXT);
	if (func == nullptr)
		throw std::runtime_error("failed to get PFN_vkCreateDebugUtilsMessengerEXT");

	if (func(Vk::instance, &create_info, nullptr, &_debug_messenger) != VK_SUCCESS)
		throw std::runtime_error("failed to create debug messenger");
}

void ge::Instance::destroy_debug_messenger(void)
{
	auto func = GET_INSTANCE_PROC(ptr, PFN_vkDestroyDebugUtilsMessengerEXT);
	if (func == nullptr)
		throw std::runtime_error("failed to get PFN_vkDestroyDebugUtilsMessengerEXT");
	func(ptr, _debug_messenger, nullptr);
}

void ge::Instance::init(void)
{
	VkInstanceCreateInfo create_info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	auto debug_info = get_debug_messenger_create_info();

	if (Vk::window.title)
		app_info.pApplicationName = Vk::window.title;

	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
	create_info.ppEnabledExtensionNames = _extensions.data();
	create_info.enabledLayerCount = static_cast<uint32_t>(_layers.size());
	create_info.ppEnabledLayerNames = _layers.data();

	if (_debug)
		create_info.pNext = &debug_info;


	if (vkCreateInstance(&create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance");

	if (_debug)
		create_debug_messenger();

	dump();

}

void ge::Instance::dump(void)
{
	std::cout << TERMINAL_COLOR_YELLOW << "Instance extensions:" << std::endl;
	std::cout << TERMINAL_COLOR_GREEN;
	for (auto& e : _extensions)
		std::cout << TAB << std::string(e) << std::endl;

	std::cout << TERMINAL_COLOR_YELLOW << "Instance layers:" << std::endl;
	std::cout << TERMINAL_COLOR_GREEN;
	for (auto& e : _layers)
		std::cout << TAB << std::string(e) << std::endl;

	std::cout << TERMINAL_COLOR_RESET;

	if (_debug_messenger)
		std::cout << TERMINAL_COLOR_CYAN << "Debug messenger creation SUCCESS" << std::endl;

	std::cout << TERMINAL_COLOR_CYAN << "Instance creation SUCCESS" << std::endl;
	std::cout << TERMINAL_COLOR_RESET;
}