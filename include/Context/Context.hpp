#pragma once

#include <vulkan/vulkan.h>

namespace gr {

	class Instance;
	class Window;
	class Device;

	class ctx {
	public:

		static void init(void);

		static void set_extent(uint32_t w, uint32_t h);
		static void use_debug(void);
		static void use_window(char const* t);
		static void use_gpu(uint32_t index);
		static void add_layer(char const* l);
		static void add_instance_extension(char const* e);
		static void add_device_extension(char const* e);

		static gr::Instance instance;
		static gr::Window window;
		static gr::Device device;

	private:

	};

}