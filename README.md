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
