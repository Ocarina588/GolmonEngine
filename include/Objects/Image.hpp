#pragma once

#include <vulkan/vulkan.h>
#include "Objects/GraphicsPipeline.hpp"

namespace ge {

	class Image {
		friend class Window;
	public:
		Image(void);
		Image(
			VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
			VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {}
		);
		Image(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED);
		~Image(void);

		operator VkImage () { return ptr; };
		operator VkImageView () { return view; };
		operator VkDeviceMemory () { return memory; };
		operator VkFramebuffer () { return framebuffer; };
		void init(
			VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties,
			VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {}
		);
		void init(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED);
		void create_framebuffer(ge::RenderPass& render_pass, VkExtent2D extent = {});

		static VkImage create_image(VkImageUsageFlags usage, VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED, VkExtent2D extent = {});
		static VkImageView create_view(VkImage image, VkImageAspectFlags aspect, VkFormat format = VK_FORMAT_UNDEFINED);
		static VkDeviceMemory create_memory(VkImage image, VkMemoryPropertyFlags properties);
		static VkFramebuffer create_framebuffer(VkImageView view, ge::RenderPass& render_pass, VkExtent2D extent = {});

		VkFormat format = VK_FORMAT_UNDEFINED;
	private:
		VkImage ptr = nullptr;
		VkImageView view = nullptr;
		VkDeviceMemory memory = nullptr;
		VkFramebuffer framebuffer = nullptr;
	};

}