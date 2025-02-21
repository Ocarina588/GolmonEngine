#include "GolmonEngine.hpp"
#include "Objects/Image.hpp"
#include "Assets/Assets.hpp"

using Vk = ge::ctx;

ge::Image::Image(void)
{

}

ge::Image::Image(Image&& i)
{
	memory = i.memory;
	ptr = i.ptr;
	framebuffer = i.framebuffer;
	view = i.view;
	format = i.format;

	i.memory = nullptr;
	i.view = nullptr;
	i.ptr = nullptr;
	i.framebuffer = nullptr;
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
	VkFormat _format, VkImageLayout layout, VkExtent2D extent, uint32_t layers)
{
	ptr = create_image(usage, _format, layout, extent, layers);
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

void ge::Image::init_raw(ge::CommandBuffer& co, void const* data, uint32_t x, uint32_t y, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties, bool submit)
{
	init(usage, aspect, properties, format, VK_IMAGE_LAYOUT_UNDEFINED, { (uint32_t)x, (uint32_t)y });

	ge::Buffer stagin_buffer;
	stagin_buffer.init(
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	stagin_buffer.map();
	stagin_buffer.memcpy(data, size);
	stagin_buffer.unmap();

	if (submit)
		co.begin();

	barrier(co, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		(uint32_t)x,
		(uint32_t)y,
		1
	};

	vkCmdCopyBufferToImage(co.ptr, stagin_buffer.ptr, ptr, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

	if (submit == false) return;
	co.end();
	ge::Fence f(false);
	co.submit(nullptr, nullptr, f);
	f.wait();
}

void ge::Image::init_with_stbi(ge::CommandBuffer& co, void const * data, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties)
{
	int x = 0, y = 0, c = 0;
	auto image_data = ge::Assets::load_from_memory((uint8_t*)data, size, &x, &y, &c, 4);
	init_raw(co, image_data, x, y, x * y * c, format, usage, aspect, properties);	
}


void ge::Image::create_framebuffer(ge::RenderPass& render_pass, VkExtent2D extent)
{
	framebuffer = create_framebuffer(view, render_pass, extent);
}

VkImage ge::Image::create_image(VkImageUsageFlags usage, VkFormat _format, VkImageLayout layout, VkExtent2D extent, uint32_t layers)
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
	create_info.arrayLayers = layers;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.initialLayout = layout;
	
	if (vkCreateImage(Vk::device, &create_info, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image");

	return image;
}

VkImageView ge::Image::create_view(VkImage image, VkImageAspectFlags aspect, VkFormat format, uint32_t base_array_layers, uint32_t layers)
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
	create_info.subresourceRange.layerCount = layers;

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

void ge::Image::barrier(CommandBuffer & buffer, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst)
{
	VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	b.oldLayout = old;
	b.newLayout = newl;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = ptr;
	b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	b.subresourceRange.baseMipLevel = 0;
	b.subresourceRange.levelCount = 1;
	b.subresourceRange.baseArrayLayer = 0;
	b.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(buffer.ptr, src, dst, 0, 0, nullptr, 0, nullptr, 1, &b);
}

void ge::Image::load_hdr(ge::CommandBuffer& co, char const* file_name)
{
	int width, height, channels;
	float* image_data = ge::Assets::read_filef(file_name, width, height, channels, 4);
	if (image_data == nullptr)
		throw std::runtime_error("failed to open " + std::string(file_name));
	else {
		init_raw(co,
			image_data, width, height, width * height * 4 * sizeof(float),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	}
}