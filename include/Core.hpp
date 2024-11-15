#pragma once

#include <imconfig.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Golmon.hpp"
#include "Model.hpp"
#include <array>
#include "Camera.hpp"

class Gizmo {
public:
	Gizmo(void)
	{
		
	}

	~Gizmo(void)
	{

	}

	void init(void);

	void draw(vulkan::CommandBuffer& b)
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(b.ptr, 0, 1, &squares.ptr, &offset);
		vkCmdDraw(b.ptr, squares.size, 1, 0, 0);
		//vkCmdBindVertexBuffers(b.ptr, 0, 1, &squares.ptr, &offset);
		//vkCmdDraw(b.ptr, squares.size, 1, 0, 0);
	}

	vulkan::Buffer lines, squares;
	vulkan::GraphicsPipeline pipeline;
};
class Core {
public:
	Core(void);
	~Core(void);

	vulkan::Buffer test;
	int main(int ac, char** av);
	void init_engine_resources(void);
	void init_app_resources(void);
	void present_image(void);

	void render_gpu(void);
	void render_cpu(void);
	void cpu_raytracing(void);
	glm::vec3 trace_ray(Ray r, int max_bounce);

	void draw(void);
	void update_dt(void);
	void update_ubo(void);

	void init_imgui(void);
	void create_gizmo(void);

	vulkan::Context context;

	vulkan::Buffer vertex_buffer, ubo_buffer, staging_buffer;
	vulkan::Image offscreen_image, depth_image;
	VkFramebuffer offscreen_buffer;
	vulkan::RenderPass render_pass;
	vulkan::GraphicsPipeline pipeline;
	vulkan::DescriptorPool descriptors;
	VkDescriptorPool imguiPool = nullptr;
	vulkan::CommandPool command_pool;
	vulkan::CommandBuffer command_buffer;
	vulkan::Semaphore image_acquired, finished_rendering;
	vulkan::Fence inflight;

	std::wstring file_loaded = L"";
	std::string file_loaded_n = "";
	bool rendering_mode = true;

	float dt = 0;
	bool clicked = false;
	Camera camera{ {0.0f, 5.0f, 5.0f}, {} };
	Camera camera_raytracing{ {0.f, 0.f, 3.5f}, {} };
	Scene scene;
	Gizmo gizmo;
};