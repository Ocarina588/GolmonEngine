#include <map>
#include "Core.hpp"

char const* model_name = "models/DamagedHelmet.glb";

Core::Core(void)
{
	ge::ctx::set_extent(1600, 900);
	ge::ctx::use_window("Vulkan App");
	ge::ctx::set_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
	ge::ctx::use_gpu(0); 
	ge::ctx::use_debug(); 
	ge::ctx::add_layer("VK_LAYER_LUNARG_monitor");
	ge::ctx::add_instance_extension("VK_EXT_swapchain_colorspace");
	ge::ctx::init();

	events.init(this);
	camera.init();

	command_pool.init(ge::ctx::device.index.graphics);
	command_buffer.init(command_pool);

	depth_image.init(
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_FORMAT_D32_SFLOAT
	);

	image_acquired.init(); finished_rendering.init();
	in_flight.init();

	render_pass.use_depth(depth_image);
	render_pass.set_clear_color({203.f, 190.f, 181.f});
	render_pass.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	render_pass.init();

	for (auto& i : ge::ctx::window.images)
		i.create_framebuffer(render_pass);

	ge::Assets::textures.emplace_back().load_hdr(command_buffer, "models/photo.hdr");
	ge::Assets::load_assimp(model_name);
	ge::Assets::upload_textures(command_buffer);

	sampler.init();

	descriptors.add_set(ge::Assets::meshes.size())
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)				// camera
		.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	// array of textures
			ge::Assets::textures.size()); 
	descriptors.init();

	descriptors.add_write(0, 0, 0, camera);
	descriptors.add_writes(0, 0, 1, ge::Assets::textures, sampler);
	descriptors.write();

	ge::Shader v("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	ge::Shader f("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	gp.add_shader_stage(v.stage);
	gp.add_shader_stage(f.stage);
	gp.set_render_pass(render_pass);
	gp.add_binding(ge::Vertex::get_binding());
	for (auto i : ge::Vertex::get_attribute())
		gp.add_attribute(i);
	for (auto i : descriptors.layouts)
		gp.add_layout(i);
	gp.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	gp.add_push_constant<ge::Assets::material_s>(VK_SHADER_STAGE_FRAGMENT_BIT);
	gp.init();

	ui.init(this);

	init_cube_map();
}

Core::~Core(void)
{
	ge::wait_idle(); 
	ge::Assets::clear();
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

			gb.bind(command_buffer);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				gb.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);
			cube_map.draw(command_buffer);

			gp.bind(command_buffer);

			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				gp.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);

			for (auto& mesh : ge::Assets::meshes) {
				vkCmdPushConstants(command_buffer, gp.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
					sizeof(ge::Assets::material_s), &ge::Assets::materials[mesh->material_id]);
				mesh->draw(command_buffer);
			}
			
			ui.render(command_buffer);
			render_pass.end(command_buffer);
		}
		command_buffer.end();
		command_buffer.submit(image_acquired, finished_rendering, in_flight);

		ge::present(finished_rendering);
		in_flight.next(); image_acquired.next(); finished_rendering.next(); command_buffer.next();
	}

	return 0;
}

void Core::updates(void)
{
	ge::update_dt();
	camera.update();
}

void Core::init_cube_map(void)
{
	std::vector<ge::Vertex> v = {
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} },  // Bottom-left
		{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} },  // Bottom-right
		{ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} },  // Top-right
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} },  // Top-left
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f} },  // Bottom-left
		{ { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f} },  // Bottom-right
		{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f} },  // Top-right
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f} },  // Top-left
	};

	std::vector<uint32_t> i = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		0, 3, 7, 0, 7, 4,
		1, 2, 6, 1, 6, 5,
		0, 1, 5, 0, 5, 4,
		3, 2, 6, 3, 6, 7,
	};

	cube_map.load_vertices(v);
	cube_map.load_indices(i);

	ge::Shader vc("shaders/cube_map.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	ge::Shader fc("shaders/cube_map.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	gb.add_shader_stage(vc.stage);
	gb.add_shader_stage(fc.stage);
	gb.set_render_pass(render_pass);
	gb.add_binding(ge::Vertex::get_binding());
	for (auto i : ge::Vertex::get_attribute())
		gb.add_attribute(i);
	for (auto i : descriptors.layouts)
		gb.add_layout(i);
	gb.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	gb.depthStencil.depthTestEnable = false;
	gb.init();
}