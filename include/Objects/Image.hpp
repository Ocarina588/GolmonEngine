#pragma once

#include <vulkan/vulkan.h>
#include "Objects/GraphicsPipeline.hpp"
#include "Objects/Commands.hpp"

namespace ge {

	class Image {
		friend class Window;
	public:
		Image(void);
		Image(
			VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
			VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {}
		);
		Image(Image&& i);
		Image(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED);
		~Image(void);

		operator VkImage () { return ptr; };
		operator VkImageView () { return view; };
		operator VkDeviceMemory () { return memory; };
		operator VkFramebuffer () { return framebuffer; };
		void init(
			VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
			VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {}, uint32_t layers = 1
		);
		void load_hdr(ge::CommandBuffer &co, char const* file_name);
		void init(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED);
		void init_raw(ge::CommandBuffer& co, void const* data, uint32_t x, uint32_t y, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties, bool submit = true);
		void init_with_stbi(ge::CommandBuffer& co, void const* data, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties);
		void create_framebuffer(ge::RenderPass& render_pass, VkExtent2D extent = {});
		void barrier(CommandBuffer& buffer, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst);

		static VkImage create_image(VkImageUsageFlags usage, VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {}, uint32_t layers = 1);
		static VkImageView create_view(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED, uint32_t base_array_layer = 0, uint32_t layers = 1);
		static VkDeviceMemory create_memory(VkImage image, VkMemoryPropertyFlags properties);
		static VkFramebuffer create_framebuffer(VkImageView view, ge::RenderPass& render_pass, VkExtent2D extent = {});

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageView view = nullptr;
	private:
		VkImage ptr = nullptr;
		VkDeviceMemory memory = nullptr;
		VkFramebuffer framebuffer = nullptr;

	};

	

}