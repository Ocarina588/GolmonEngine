#pragma once

#include "Context/Window.hpp"
#include "Context/Instance.hpp"
#include "Context/Device.hpp"
#include "utils.hpp"

namespace gr {

	class Context {
	public:

		static void init(void);

		static void use_debug(void);
		static void use_window(int w, int h, char const* t);
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