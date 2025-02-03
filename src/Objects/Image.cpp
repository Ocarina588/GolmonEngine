#include "GolmonRenderer.hpp"
#include "Objects/Image.hpp"

using Vk = gr::Context;

gr::Image::Image(void)
{

}

gr::Image::~Image(void)
{

}

void gr::Image::init(void)
{
	VkImageCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

}

VkImage gr::Image::create_image(void)
{
	VkImage image = nullptr;
	return image;
}

VkImageView gr::Image::create_view(VkImage image, VkFormat format, VkImageAspectFlags aspect)
{
	VkImageViewCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	VkImageView view = nullptr;

	create_info.image = image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = format;
	create_info.subresourceRange.aspectMask = aspect;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(Vk::device.ptr, &create_info, nullptr, &view) != VK_SUCCESS) 
		throw std::runtime_error("failed to create image view");

	return view;
}

VkDeviceMemory gr::Image::create_memory(void)
{
	VkDeviceMemory memory = nullptr;
	return memory;
}