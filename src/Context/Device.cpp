#include <unordered_set>
#include "GolmonEngine.hpp"

using Vk = ge::ctx;

ge::Device::Device(void)
{

}

ge::Device::~Device(void)
{
	vkDestroyDevice(ptr, nullptr);
}

void ge::Device::init(void)
{
	VkDeviceCreateInfo create_info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	uint32_t count = 0;



	vkEnumeratePhysicalDevices(Vk::instance, &count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(count);
	vkEnumeratePhysicalDevices(Vk::instance, &count, physical_devices.data());

	if (_gpu > count)
		throw std::runtime_error("chosen gpu index out of bound");

	physical_ptr = physical_devices[_gpu]; 

	get_info(); 

	auto queue_infos = choose_queues();

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = VK_TRUE;

	_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
	indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE; 
	indexing_features.runtimeDescriptorArray = VK_TRUE;

	VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features2.features = features;
	features2.pNext = &indexing_features;

	create_info.enabledExtensionCount = static_cast<uint32_t>(_extensions.size()); 
	create_info.ppEnabledExtensionNames = _extensions.data();
	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
	create_info.pQueueCreateInfos = queue_infos.data();
	create_info.pNext = &features2;

	if (vkCreateDevice(physical_ptr, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create device");

	vkGetDeviceQueue(ptr, index.graphics, 0, &queue.graphics);
	vkGetDeviceQueue(ptr, index.compute, 0, &queue.compute);
	vkGetDeviceQueue(ptr, index.transfer, 0, &queue.transfer);
	if (Vk::window.ptr)
		vkGetDeviceQueue(ptr, index.present, 0, &queue.present);

	dump();

}

void ge::Device::get_info(void)
{
	uint32_t count = 0;

	vkGetPhysicalDeviceProperties(physical_ptr, &properties);
	vkGetPhysicalDeviceFeatures(physical_ptr, &features);
	vkGetPhysicalDeviceMemoryProperties(physical_ptr, &memory_properties);

	vkGetPhysicalDeviceQueueFamilyProperties(physical_ptr, &count, nullptr);
	family_properties.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_ptr, &count, family_properties.data());

	if (Vk::window.ptr == nullptr) return;

	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_ptr, Vk::window.surface, &Vk::window.capabilites) != VK_SUCCESS)
		throw std::runtime_error("failed to get surface capabilites");
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_ptr, Vk::window.surface, &count, nullptr);
	Vk::window.formats.resize(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_ptr, Vk::window.surface, &count, Vk::window.formats.data());
}

std::vector<VkDeviceQueueCreateInfo> ge::Device::choose_queues(void)
{
	for (uint32_t i = 0; i < family_properties.size(); i++) {
		if (index.graphics == ~(0u) && family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			index.graphics = i;
		if (index.compute == ~(0u) && family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			index.compute = i;
		if (index.transfer == ~(0u) && family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			index.transfer = i;

		if (Vk::window.ptr == nullptr) continue;

		VkBool32 support = false;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(physical_ptr, i, Vk::window.surface, &support) != VK_SUCCESS)
			throw std::runtime_error("failed to get surface support");
		if (index.present == ~(0u) && support)
			index.present = i;
	}

	std::unordered_set<uint32_t> s = {
		index.graphics, index.compute, index.transfer
	};
	if (Vk::window.ptr)
		s.insert(index.present);

	std::vector<VkDeviceQueueCreateInfo> create_infos;

	for (auto& i : s) {
		create_infos.push_back({
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = i,
			.queueCount = 1,
			.pQueuePriorities = &priority
		});
	}

	return create_infos;
}

void ge::Device::dump(void)
{
	std::cout << TERMINAL_COLOR_YELLOW;
	std::cout << "Device extensions:" << std::endl;
	
	std::cout << TERMINAL_COLOR_GREEN;
	for (auto& e : _extensions)
		std::cout << TAB << e << std::endl;

	std::cout << TERMINAL_COLOR_CYAN;
	std::cout << "Device creation SUCCESS [" << TERMINAL_COLOR_MAGENTA << std::string(properties.deviceName);
	std::cout << TERMINAL_COLOR_CYAN << "]" << std::endl;
	std::cout << TERMINAL_COLOR_RESET;
}

// FUNCTIONS 

uint32_t ge::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	auto const &m = ctx::device.memory_properties;

	for (uint32_t i = 0; i < m.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && ((m.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}

	throw std::runtime_error("no memory type");
	return 0;
}

VkMemoryRequirements ge::get_memory_requirements(VkImage image)
{
	VkMemoryRequirements r;
	vkGetImageMemoryRequirements(ctx::device, image, &r);
	return r;
}

VkMemoryRequirements ge::get_memory_requirements(VkBuffer buffer)
{
	VkMemoryRequirements r;
	vkGetBufferMemoryRequirements(ctx::device, buffer, &r);
	return r;
}

void ge::wait_idle(void)
{
	vkDeviceWaitIdle(ge::ctx::device);
}