#include "Objects/CubeMap.hpp"
#include "Assets/Assets.hpp"
//#include "Objects/Image.hpp"

ge::CubeMap::CubeMap(void)
{
}

ge::CubeMap::~CubeMap(void)
{

}

void ge::CubeMap::init(char const* file_name)
{
	int width, height, channels;
	ge::Assets::read_filef(file_name, width, height, channels, 4);

	image = ge::Image::create_image(
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		{ (uint32_t)width, (uint32_t)height },
		6
	); 
	memory = ge::Image::create_memory(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkBindImageMemory(ctx::device.ptr, image, memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind image");


	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	render_pass.create_info.attachmentCount = 1;
	render_pass.create_info.pAttachments = &colorAttachment;
	render_pass.create_info.subpassCount = 1;
	render_pass.create_info.pSubpasses = &subpass;

	render_pass.init(render_pass.create_info);

	for (int i = 0; i < 6; i++) {
		view[i] = ge::Image::create_view(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, i, 1);
		fb[i] = ge::Image::create_framebuffer(view[i], render_pass, { (uint32_t)width, (uint32_t)height });
	}

}