#include "Objects/CubeMap.hpp"
#include "Assets/Assets.hpp"
#include "Objects/Shader.hpp"
//#include "Objects/Image.hpp"

ge::CubeMap::CubeMap(void)
{
}

ge::CubeMap::~CubeMap(void)
{

}

void ge::CubeMap::init(char const* file_name, ge::CommandBuffer &co, ge::RenderPass &render_pass)
{
	//int width, height, channels;
	//ge::Assets::read_filef(file_name, width, height, channels, 4);

	//image = ge::Image::create_image(
	//	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	//	VK_FORMAT_R32G32B32A32_SFLOAT,
	//	VK_IMAGE_LAYOUT_UNDEFINED,
	//	{ (uint32_t)width, (uint32_t)height },
	//	6
	//); 
	//memory = ge::Image::create_memory(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//if (vkBindImageMemory(ctx::device.ptr, image, memory, 0) != VK_SUCCESS)
	//	throw std::runtime_error("failed to bind image");

	//for (int i = 0; i < 6; i++) {
	//	view[i] = ge::Image::create_view(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, i, 1);
	//	fb[i] = ge::Image::create_framebuffer(view[i], render_pass, { (uint32_t)width, (uint32_t)height });
	//}



	init_vertices(co);

}

void ge::CubeMap::init_vertices(ge::CommandBuffer& co)
{

}