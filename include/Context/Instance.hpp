#pragma once

#include <vulkan/vulkan.h>

namespace gr {

	class Instance {
		friend class ctx;
		friend class Window;
	public:
		Instance(void);
		~Instance(void);

		operator VkInstance() { return ptr; }
		void init(void);
	
		VkInstance ptr = nullptr;
		VkApplicationInfo app_info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		
	private:

		void create_debug_messenger(void);
		void destroy_debug_messenger(void);

		void dump(void);
		
		bool _debug = false;
		std::vector<char const*> _extensions;
		std::vector<char const*> _layers;
		VkDebugUtilsMessengerEXT _debug_messenger = nullptr;
	};
}