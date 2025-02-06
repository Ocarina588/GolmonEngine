#include "Core.hpp"

Core::Core(void)
{
	ge::ctx::set_extent(1280, 720);
	ge::ctx::use_window("Vulkan App");
	ge::ctx::set_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
	ge::ctx::use_gpu(0); 
	ge::ctx::use_debug(); 
	ge::ctx::add_layer("VK_LAYER_LUNARG_monitor");
	ge::ctx::init();

	events.init(this);
	camera.init();
	mesh.load_from_glb("models/DamagedHelmet.glb");

	depth_image.init(
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_FORMAT_D32_SFLOAT
	);

	image_acquired.init(); finished_rendering.init();
	in_flight.init();

	command_pool.init(ge::ctx::device.index.graphics);
	command_buffer.init(command_pool);

	render_pass.use_depth(depth_image);
	render_pass.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	render_pass.init();

	for (auto& i : ge::ctx::window.images)
		i.create_framebuffer(render_pass);

	descriptors.add_set(1)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	descriptors.init();

	descriptors.add_write(0, 0, 0, camera);
	descriptors.write();

	ge::Shader v("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	ge::Shader f("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	gp.add_shader_stage(v.stage);
	gp.add_shader_stage(f.stage);
	gp.set_render_pass(render_pass);
	gp.add_binding(ge::Vertex::get_binding());
	for (auto i : ge::Vertex::get_attribute())
		gp.add_attribute(i);
	gp.add_layout(descriptors.layouts[0]);
	gp.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	gp.init();
}

Core::~Core(void)
{
	ge::wait_idle(); 
}

int Core::main(int ac, char **av)
{
	while (ge::ctx::window.is_open()) {
		ge::ctx::window.poll_events();
		in_flight.wait();
		updates();
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
	return 0;
}

void Core::updates(void)
{
	ge::update_dt();
	camera.update();
}