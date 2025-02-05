#pragma once

#include <GLFW/glfw3.h>

namespace ge {

	class Events {
	public:
		Events(void);
		~Events(void);

		void init(void* p);

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		static void scroll_callback(GLFWwindow* window, double xpos, double ypos);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		bool clicked = false;
	};

}