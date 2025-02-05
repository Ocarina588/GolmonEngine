#include <iostream>
#include <glm/glm.hpp>
#include <array>
#include "GolmonEngine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Core.hpp"

/*
struct Vertex {
	glm::vec3 pos;
	static VkVertexInputBindingDescription get_binding(void)
	{
		VkVertexInputBindingDescription binding{};

		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex);

		return binding;
	}
	static VkVertexInputAttributeDescription get_attribute(void)
	{
		VkVertexInputAttributeDescription attribute{};
		attribute.binding = 0;
		attribute.offset = offsetof(Vertex, pos);
		attribute.format = VK_FORMAT_R32G32B32_SFLOAT;

		return attribute;
	}
};

struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//Vertex positions[3] = {
//	{{0.0f, -0.5f}},
//	{{0.5f, 0.5f}},
//	{{0.2f, 0.5f}},
//};
//
//uint32_t indices[3] = {
//	0, 1, 2
//};

void core(int ac, char** av)
{
	ge::ctx::set_extent(1280, 720);
	ge::ctx::use_window("Vulkan App");
	ge::ctx::use_gpu(0);
	ge::ctx::use_debug();
	ge::ctx::add_layer("VK_LAYER_LUNARG_monitor");
	ge::ctx::init();

	ge::Image depth_image(
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_FORMAT_D32_SFLOAT
	);

	ge::Semaphore image_acquired, finished_rendering;
	ge::Fence in_flight;

	ge::CommandPool command_pool(ge::ctx::device.index.graphics);
	ge::CommandBuffer command_buffer(command_pool);

	ge::RenderPass render_pass;
	render_pass.use_depth(depth_image);
	render_pass.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	render_pass.init();

	for (auto& i : ge::ctx::window.images)
		i.create_framebuffer(render_pass);

	ge::Camera camera;

	ge::DescriptorPool descriptors;
	descriptors.add_set(1)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	descriptors.init();

	descriptors.add_write(0, 0, 0, camera);
	descriptors.write();

	ge::Shader v("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	ge::Shader f("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	ge::GraphicsPipeline gp;
	gp.add_shader_stage(v.stage);
	gp.add_shader_stage(f.stage);
	gp.set_render_pass(render_pass);
	gp.add_binding(Vertex::get_binding());
	gp.add_attribute(Vertex::get_attribute());
	gp.add_layout(descriptors.layouts[0]);
	gp.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	gp.init(); 

	ge::Mesh mesh;
	mesh.load_from_glb("models/DamagedHelmet.glb");

	while (ge::ctx::window.is_open()) {
		ge::ctx::window.poll_events();
		in_flight.wait();
		ge::acquire_next_image(image_acquired);

		command_buffer.begin();
		{
			render_pass.begin(command_buffer);
			gp.bind(command_buffer);

			vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, 
				gp.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);

			mesh.draw(command_buffer);
			render_pass.end(command_buffer);
		}
		command_buffer.end();
		command_buffer.submit(image_acquired, finished_rendering, in_flight);

		ge::present(finished_rendering);
	}
	ge::wait_idle();
}

*/

int main(int ac, char** av)
{
	try {
		Core core;
		return core.main(ac, av);
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}