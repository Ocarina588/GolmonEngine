#include "GolmonEngine.hpp"
#include "Core.hpp"
#include <GLFW/glfw3.h>

ge::Events::Events(void)
{

}

ge::Events::~Events(void)
{

}

void ge::Events::init(void* p)
{
	glfwSetWindowUserPointer(ge::ctx::window, p);
	glfwSetMouseButtonCallback(ge::ctx::window, ge::Events::mouse_button_callback);
	glfwSetScrollCallback(ge::ctx::window, ge::Events::scroll_callback);
	glfwSetCursorPosCallback(ge::ctx::window, ge::Events::mouse_callback);
	glfwSetKeyCallback(ge::ctx::window, ge::Events::key_callback);
}

void ge::Events::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Core& core = *(Core*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_TAB:
			//core.rendering_mode = !core.rendering_mode;
		default:
			break;
		}
	} else if (action == GLFW_RELEASE) {
	}
}

void ge::Events::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double old_xpos = 0;
	static double old_ypos = 0;
	Core& core = *(Core*)glfwGetWindowUserPointer(ge::ctx::window);

	if (core.events.clicked)
		core.camera.process_mouse(xpos - old_xpos, ypos - old_ypos);
	
	old_xpos = xpos;
	old_ypos = ypos;
}

void ge::Events::scroll_callback(GLFWwindow* window, double xpos, double ypos)
{
	Core& core = *(Core*)glfwGetWindowUserPointer(window);

	core.camera.process_scroll(xpos, ypos);
}

void ge::Events::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Core& core = *(Core*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		core.events.clicked = true;
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		core.events.clicked = false;
}
