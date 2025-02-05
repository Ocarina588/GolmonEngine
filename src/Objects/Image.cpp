#include "GolmonEngine.hpp"
#include "Objects/Image.hpp"

using Vk = ge::ctx;

ge::Image::Image(void)
{

}

ge::Image::Image(VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
	VkFormat _format, VkImageLayout layout, VkExtent2D extent)
{
	init(usage, aspect, properties, _format, layout, extent);
}

ge::Image::Image(VkImage image, VkImageAspectFlags aspect, VkFormat format)
{
	init(image, aspect, format);
}

ge::Image::~Image(void)
{
	if (memory) {
		vkFreeMemory(Vk::device, memory, nullptr);
		vkDestroyImage(Vk::device, ptr, nullptr);
	}
	if (framebuffer)
		vkDestroyFramebuffer(Vk::device, framebuffer, nullptr);
	vkDestroyImageView(Vk::device, view, nullptr);
}

void ge::Image::init(VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
	VkFormat _format, VkImageLayout layout, VkExtent2D extent)
{
	ptr = create_image(usage, _format, layout, extent);
	memory = create_memory(ptr, properties);

	if (vkBindImageMemory(ctx::device.ptr, ptr, memory, 0) != VK_SUCCESS) 
		throw std::runtime_error("failed to bind image");

	view = create_view(ptr, aspect, _format);
	format = (_format == VK_FORMAT_UNDEFINED) ? Vk::device.format.format : _format;
}

void ge::Image::init(VkImage image, VkImageAspectFlags aspect, VkFormat format)
{
	ptr = image;
	view = create_view(image, aspect, format);
}

void ge::Image::create_framebuffer(ge::RenderPass& render_pass, VkExtent2D extent)
{
	framebuffer = create_framebuffer(view, render_pass, extent);
}

VkImage ge::Image::create_image(VkImageUsageFlags usage, VkFormat _format, VkImageLayout layout, VkExtent2D extent)
{
	VkImageCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	VkImage image = nullptr;

	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.format = (_format == VK_FORMAT_UNDEFINED) ? Vk::device.format.format : _format;
	if (extent.width == 0 && extent.height == 0) {
		create_info.extent.width = Vk::device.extent.width;
		create_info.extent.height = Vk::device.extent.height;
	} else {
		create_info.extent.width = extent.width;
		create_info.extent.height = extent.height;
	}
	create_info.extent.depth = 1;
	create_info.mipLevels = 1;
	create_info.arrayLayers = 1;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.initialLayout = layout;
	
	if (vkCreateImage(Vk::device, &create_info, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image");

	return image;
}

VkImageView ge::Image::create_view(VkImage image, VkImageAspectFlags aspect, VkFormat format)
{
	VkImageView view = nullptr;
	VkImageViewCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

	create_info.image = image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = (format == VK_FORMAT_UNDEFINED) ? Vk::device.format.format : format;
	create_info.components = {};
	create_info.subresourceRange.aspectMask = aspect;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(Vk::device, &create_info, nullptr, &view) != VK_SUCCESS)
		throw std::runtime_error("failed to create image view");

	return view;
}

VkDeviceMemory ge::Image::create_memory(VkImage image, VkMemoryPropertyFlags properties)
{
	VkMemoryAllocateInfo info{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	VkDeviceMemory memory = nullptr;
	auto m = ge::get_memory_requirements(image);

	info.allocationSize = m.size;
	info.memoryTypeIndex = ge::find_memory_type(m.memoryTypeBits, properties);

	if (vkAllocateMemory(ctx::device, &info, nullptr, &memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");

	return memory;
}

VkFramebuffer ge::Image::create_framebuffer(VkImageView view, ge::RenderPass &render_pass, VkExtent2D extent)
{
	VkFramebufferCreateInfo create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	VkFramebuffer framebuffer = nullptr;
	VkImageView attachments[] = { view, nullptr };

	if (render_pass.depth_image)
		attachments[1] = render_pass.depth_image->view;

	create_info.renderPass = render_pass.ptr;
	create_info.attachmentCount = 1 + (render_pass.depth_image != nullptr);
	create_info.pAttachments = attachments;
	create_info.layers = 1;

	if (extent.width == 0 && extent.height == 0) {
		create_info.width = ctx::device.extent.width;
		create_info.height = ctx::device.extent.height;
	}
	else {
		create_info.width = extent.width;
		create_info.height = extent.height;
	}

	if (vkCreateFramebuffer(ctx::device.ptr, &create_info, nullptr, &framebuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create framebuffer");

	return framebuffer;
}