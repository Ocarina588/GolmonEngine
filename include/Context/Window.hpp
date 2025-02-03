#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace gr {

	class Window {
		friend class Context;
		friend class Instance;

	public:
		Window(void);
		~Window(void);

		operator GLFWwindow * () { return ptr; }

		VkExtent2D get_extent(void);

		inline bool is_open(void) const noexcept { 
			if (ptr == nullptr) return false; 
			else return glfwWindowShouldClose(ptr) == false; 
		}
		inline void poll_events(void) const noexcept {
			glfwPollEvents();
		}

		GLFWwindow* ptr = nullptr;
		VkSurfaceKHR surface = nullptr;
		VkSwapchainKHR swapchain = nullptr;

		VkSurfaceCapabilitiesKHR capabilites{};
		VkPresentModeKHR mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		std::vector<VkSurfaceFormatKHR> formats;

		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		std::vector<VkFramebuffer> framebuffers;


	private:
		void dump(void);
		void dump_swapchain(void);
		void init(int w, int h, char const* t);
		void init_surface(void);
		void init_swapchain(void);
		void init_swapchain_resources(void);

		std::vector<char const*> get_required_extensions(void);

		char const* title = nullptr;


	};
}