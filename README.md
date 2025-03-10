# Golmon Renderer
The purpose of this project is to create a __Vulkan__ Renderer in __C++__ without sacrificing performance on the abstraction.

### Hello Triangle example (without vertex buffer)

The vertices information are hard-coded in the vertex shader.


```c++
int main(int ac, char** av)
{
	gr::ctx::set_extent(1280, 720);
	gr::ctx::use_window("Vulkan App");
	gr::ctx::use_gpu(0);
	gr::ctx::use_debug();
	gr::ctx::add_layer("VK_LAYER_LUNARG_monitor");
	gr::ctx::init();

	gr::Image depth_image(
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_FORMAT_D32_SFLOAT
	);

	gr::Semaphore image_acquired, finished_rendering;
	gr::Fence in_flight;

	gr::CommandPool command_pool(gr::ctx::device.index.graphics);
	gr::CommandBuffer command_buffer(command_pool);

	gr::RenderPass render_pass;
	render_pass.use_depth(depth_image);
	render_pass.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	render_pass.init();

	for (auto& i : gr::ctx::window.images)
		i.create_framebuffer(render_pass);

	gr::Shader v("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	gr::Shader f("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	gr::GraphicsPipeline gp;
	gp.add_shader_stage(v.stage);
	gp.add_shader_stage(f.stage);
	gp.set_render_pass(render_pass);
	gp.init();

	while (gr::ctx::window.is_open()) {
		gr::ctx::window.poll_events();
		in_flight.wait();
		gr::acquire_next_image(image_acquired);

		command_buffer.begin();
		{
			render_pass.begin(command_buffer);
			gp.bind(command_buffer);
			vkCmdDraw(command_buffer.ptr, 3, 1, 0, 0);
			render_pass.end(command_buffer);
		}
		command_buffer.end();
		command_buffer.submit(image_acquired, finished_rendering, in_flight);

		gr::present(finished_rendering);
	}
	gr::wait_idle();
}

```

### Loading GLTF (.glb) model

```c++

#include "GolmonEngine.hpp"

class Core {
public:

	Core(void);
	~Core(void);

	int main(int ac, char** av);
	void updates(void);

	ge::Events events;
	ge::Camera camera;
	ge::Mesh mesh;
	ge::Sampler sampler;

	ge::Image depth_image;
	ge::Semaphore image_acquired, finished_rendering;
	ge::Fence in_flight;
	ge::CommandPool command_pool;
	ge::CommandBuffer command_buffer;
	ge::RenderPass render_pass;
	ge::DescriptorPool descriptors;
	ge::GraphicsPipeline gp;
};

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
		i.create_framebuffer(render_pass);;

	ge::Assets::load_glb("models/DamagedHelmet.glb");
	ge::Assets::init_materials(command_buffer);

	sampler.init();

	descriptors.add_set(1)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)				// camera
		.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// albedo
		.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// normal
		.add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// metallic
		.add_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)	// emissive
		.add_binding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);	// occlusion
	descriptors.init();

	descriptors.add_write(0, 0, 0, camera);
	descriptors.add_write(0, 0, 1, ge::Assets::materials[0].albedo,		VK_IMAGE_LAYOUT_GENERAL, sampler);
	descriptors.add_write(0, 0, 2, ge::Assets::materials[0].normal,		VK_IMAGE_LAYOUT_GENERAL, sampler);
	descriptors.add_write(0, 0, 3, ge::Assets::materials[0].metallic,	VK_IMAGE_LAYOUT_GENERAL, sampler);
	descriptors.add_write(0, 0, 4, ge::Assets::materials[0].emissive,	VK_IMAGE_LAYOUT_GENERAL, sampler);
	descriptors.add_write(0, 0, 5, ge::Assets::materials[0].occlusion,	VK_IMAGE_LAYOUT_GENERAL, sampler);
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

			ge::Assets::meshes["models/DamagedHelmet.glb"]->draw(command_buffer);
			
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

```