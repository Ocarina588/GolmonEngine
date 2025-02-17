#include <map>
#include "Core.hpp"

char const* model_name = "models/DamagedHelmet.glb";
//char const* model_name = "models/untitled.glb";
//char const* model_name = "models/untitled.glb";

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

	command_pool.init(ge::ctx::device.index.graphics);
	command_buffer.init(command_pool);

	depth_image.init(
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_FORMAT_D32_SFLOAT
	);

	//background.init_with_stbif(command_buffer, "models/background.hdr",
	//	VK_FORMAT_R16G16B16A16_SFLOAT,
	//	VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	//);

	image_acquired.init(); finished_rendering.init();
	in_flight.init();

	render_pass.use_depth(depth_image);
	//render_pass.set_clear_color({203.f, 190.f, 181.f});
	render_pass.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	render_pass.init();

	for (auto& i : ge::ctx::window.images)
		i.create_framebuffer(render_pass);

	ge::Assets::load_assimp(model_name);
	ge::Assets::upload_textures(command_buffer);
	//ge::Assets::load_glb(model_name);
	//ge::Assets::init_materials(command_buffer);

	sampler.init();

	descriptors.add_set(ge::Assets::meshes.size())
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)				// camera
		.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// albedo
		.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) 	// normal
		.add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) 	// metallic
		.add_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// emissive
		.add_binding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); 	// occlusion
		//.add_binding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);	// background
	descriptors.init();

	for (int i = 0; i < ge::Assets::meshes.size(); i++) {
		auto const& mesh = ge::Assets::meshes[i];
		auto const& material = ge::Assets::materials[mesh->material_id];

		descriptors.add_write(0, i, 0, camera);
		descriptors.add_write(0, i, 1, material.index_albedo ? (VkImageView)ge::Assets::textures[material.index_albedo - 1] : nullptr, VK_IMAGE_LAYOUT_GENERAL, sampler);
		descriptors.add_write(0, i, 2, material.index_normal ? (VkImageView)ge::Assets::textures[material.index_normal - 1] : nullptr, VK_IMAGE_LAYOUT_GENERAL, sampler);
		descriptors.add_write(0, i, 3, material.index_metallic ? (VkImageView)ge::Assets::textures[material.index_metallic - 1] : nullptr, VK_IMAGE_LAYOUT_GENERAL, sampler);
		descriptors.add_write(0, i, 4, material.index_emissive ? (VkImageView)ge::Assets::textures[material.index_emissive - 1] : nullptr, VK_IMAGE_LAYOUT_GENERAL, sampler);
		descriptors.add_write(0, i, 5, material.index_occlusion ? (VkImageView)ge::Assets::textures[material.index_occlusion - 1] : nullptr, VK_IMAGE_LAYOUT_GENERAL, sampler);
		//descriptors.add_write(0, i, 6, background, VK_IMAGE_LAYOUT_GENERAL, sampler);
	}
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
	gp.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	gp.init();

	ui.init(render_pass.ptr);;
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
			gp.bind(command_buffer);

			vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS,
				gp.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);

			for (auto& mesh : ge::Assets::meshes) {
				*(ge::Assets::material_s*)&camera.ubo.index_albedo = ge::Assets::materials[mesh->material_id];
				camera.write_ubo();
				mesh->draw(command_buffer);
			}
			
			ui.render(command_buffer.ptr);
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
