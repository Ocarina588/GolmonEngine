#include "Core.hpp"
#include "chrono"
#include <Windows.h>

using Vk = vulkan::Context;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

std::vector<Vertex> vertices = {
	{{0.0f, -0.5f, 1.f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}
};

Core::Core(void)
{
	rendering_mode = true;

	init_engine_resources();
	init_app_resources();

	glfwSetWindowUserPointer(Vk::window.ptr, this);
	glfwSetKeyCallback(Vk::window.ptr, keyCallback);

	init_imgui();

	//scene.load_obj("models/cube.obj");
} 

Core::~Core(void)
{

}

void Core::init_engine_resources(void)
{
	Vk::window.init(640, 360, "Vulkan App");
	Vk::instance.add_layer("VK_LAYER_LUNARG_monitor");

	context.init(true);

	finished_rendering.init();
	image_acquired.init();
	inflight.init(true);
	command_pool.init(Vk::device.index.graphics);
	command_buffer.init(command_pool);

	depth_image.init(
		context.window.extent,
		VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	offscreen_image.init(
		context.window.extent,
		Vk::window.format.format , VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	render_pass.use_depth(depth_image); 
}

void Core::init_app_resources(void)
{
	//RENDER PASS
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	render_pass.init();
	Vk::window.init_framebuffers(render_pass);

	VkImageView views[] = { offscreen_image.view, depth_image.view };
	VkFramebufferCreateInfo create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	create_info.attachmentCount = 2;
	create_info.pAttachments = views;
	create_info.width = Vk::window.extent.width;
	create_info.height = Vk::window.extent.height;
	create_info.renderPass = render_pass.ptr;
	create_info.layers = 1;
	
	if (vkCreateFramebuffer(Vk::device.ptr, &create_info, nullptr, &offscreen_buffer) != VK_SUCCESS) throw std::runtime_error("failed to create framebuffer");

	//BUFFERS
	vertex_buffer.init(
		(void*)vertices.data(),
		static_cast<uint32_t>(sizeof(Vertex) * vertices.size()),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	staging_buffer.init(
		sizeof(uint32_t) * Vk::window.extent.width * Vk::window.extent.height,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	staging_buffer.map();
	std::memset(staging_buffer.data, 0, staging_buffer.size);

	ubo_buffer.init(
		static_cast<uint32_t>(sizeof(UBO)),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	ubo_buffer.map();

	//DESCRIPTORS
	descriptors.add_set(1)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	descriptors.init();

	descriptors.add_write(0, 0, 0, ubo_buffer.ptr);
	descriptors.write();

	//PIPELINE
	vulkan::Shader vertex("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vulkan::Shader fragment("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	pipeline.add_shader_stage(vertex.stage);
	pipeline.add_shader_stage(fragment.stage);
	pipeline.set_render_pass(render_pass);
	pipeline.add_binding(Vertex::getBindingDescription());
	for (auto i : Vertex::getAttributeDescriptions())
		pipeline.add_attribute(i);
	pipeline.add_layout(descriptors.layouts[0]);

	pipeline.init();
}

void Core::init_imgui(void)
{
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(Vk::window.ptr, true);

	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	if (vkCreateDescriptorPool(Vk::device.ptr, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Vk::instance.ptr;
	init_info.PhysicalDevice = Vk::device.physical_ptr;
	init_info.Device = Vk::device.ptr;
	init_info.Queue = Vk::device.queue.graphics;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = Vk::window.surface_capabilities.minImageCount;
	init_info.ImageCount = Vk::window.surface_capabilities.minImageCount + 1;
	init_info.RenderPass = render_pass.ptr;
	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();
}

#include <windows.h>
#include <iostream>
#include <string>

#include <windows.h>
#include <iostream>
#include <string>

HANDLE SetupFileChangeNotification(const std::wstring& directory) {
	HANDLE hChangeHandle = FindFirstChangeNotification(
		directory.c_str(),
		TRUE,                           // Do not watch subdirectories
		FILE_NOTIFY_CHANGE_LAST_WRITE);  // Watch for file modifications

	if (hChangeHandle == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to set up directory change notification." << std::endl;
	}

	return hChangeHandle;
}

bool CheckFileTimestampChanged(const std::wstring& filepath, FILETIME& lastWriteTime) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if (GetFileAttributesEx(filepath.c_str(), GetFileExInfoStandard, &fileInfo)) {
		// Check if the last write time has changed
		if (CompareFileTime(&fileInfo.ftLastWriteTime, &lastWriteTime) != 0) {
			// Update last write time to the latest time
			lastWriteTime = fileInfo.ftLastWriteTime;
			return true;  // File has been modified
		}
	}
	return false;  // No change in file timestamp
}

bool CheckFileChanged(HANDLE hChangeHandle, const std::wstring& filepath, FILETIME& lastWriteTime) {
	DWORD waitStatus = WaitForSingleObject(hChangeHandle, 0);  // Non-blocking wait

	if (waitStatus == WAIT_OBJECT_0) {
		// A change in the directory was detected, check the specific file's timestamp
		bool fileChanged = CheckFileTimestampChanged(filepath, lastWriteTime);

		// Reset the directory change notification for future monitoring
		FindNextChangeNotification(hChangeHandle);

		return fileChanged;
	}

	return false;
}

int Core::main(int ac, char** av)
{
	HANDLE hChangeHandle = SetupFileChangeNotification(L"./");
	if (hChangeHandle == INVALID_HANDLE_VALUE) {
		return 1;  // Exit if unable to set up change notification
	}
	bool tmp = false;
	// Get the initial last write time of the file
	FILETIME lastWriteTime = { 0 };

	while (Vk::window.should_close() == false) {

		if (tmp == false && file_loaded.empty() == false) {
			tmp = true;
			CheckFileTimestampChanged(file_loaded, lastWriteTime);
		}

		Vk::window.poll_events();;
		inflight.wait();
		Vk::window.acquire_next_image(image_acquired.ptr, nullptr);
		
		if (CheckFileChanged(hChangeHandle, file_loaded, lastWriteTime)) {
			std::wcout << L"File modified: " << "filename" << std::endl;
			scene.load_obj(file_loaded_n.c_str());
		}

		if (rendering_mode)
			render_gpu();
		else 
			render_cpu();

		command_buffer.submit(image_acquired, finished_rendering, inflight);
		
		Vk::window.present(finished_rendering);
	}

	return 0;
}

void Core::update_ubo(void)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	//time = 1.f;

	UBO ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//ubo.model = glm::1rotate(ubo.model, time * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));

	ubo.view = glm::lookAt(glm::vec3(0.0f, -5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.proj = glm::perspective(glm::radians(45.0f), Vk::window.extent.width / (float)Vk::window.extent.height, 0.1f, 10.0f);

	ubo.proj[1][1] *= -1;

	ubo_buffer.memcpy(&ubo, sizeof(ubo));
}

std::string ask_file(char const* filter)
{
	OPENFILENAMEA ofn;
	char filename[256] = { 0 };
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 256;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "";

	std::string file;

	if (GetOpenFileNameA(&ofn))
		file = filename;
	return file;
}

#include <string>
#include <locale>
#include <codecvt>

std::wstring StringToWString(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
	std::wstring wstr(size_needed, L'\0'); // Create a wstring of the required size
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
	return wstr;
}

void Core::render_gpu(void)
{
	static std::string file_to_load = "";

	if (file_to_load != "") {
		scene.load_obj(file_to_load.c_str());
		file_loaded = StringToWString(file_to_load);
		file_loaded_n = file_to_load;
	}
	file_to_load = "";

	command_buffer.begin(true);
	{
		render_pass.begin(command_buffer, Vk::window.extent, offscreen_buffer);
		pipeline.bind(command_buffer);

		update_ubo();

		vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);
		
		//VkDeviceSize offset = 0;
		//vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_buffer.ptr, &offset);
		//vkCmdDraw(command_buffer.ptr, 3, 1, 0, 0);

		scene.draw(command_buffer);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();


		if (ImGui::BeginMainMenuBar()) {


			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open file")) file_to_load = ask_file("Obj files (*.obj)\0*.obj\0").c_str();
				ImGui::EndMenu();
			}

			//if (ImGui::BeginMenu("Project")) {
			//	if (ImGui::MenuItem("Terrain")) {
			//		terrain = std::make_shared<engine::Terrain>(std::make_shared<engine::Perlin>(std::time(NULL)));
			//		mode = true;
			//	}
			//	if (ImGui::MenuItem("Rubik's Cube"))
			//		mode = false;
			//	ImGui::EndMenu();

			//}

			if (ImGui::BeginMenu("Camera")) {
				if (ImGui::MenuItem("Scene"));
				if (ImGui::MenuItem("Player"));
				ImGui::EndMenu();
			}


			ImGui::EndMainMenuBar();
		}


		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.ptr);

		render_pass.end(command_buffer);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		vulkan::Image::cpy(command_buffer, Vk::window.extent, offscreen_image.image, Vk::window.images[Vk::window.image_index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
	}
	command_buffer.end();
}

void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0; // No row length
	region.bufferImageHeight = 0; // No image height
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0; // Base mip level
	region.imageSubresource.baseArrayLayer = 0; // First layer
	region.imageSubresource.layerCount = 1; // One layer
	region.imageOffset = { 0, 0, 0 }; // Copy to the top-left corner
	region.imageExtent = { width, height, 1 }; // Width, height, and depth

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Core::render_cpu(void)
{
	cpu_raytracing();

	command_buffer.begin();
	{
		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		copyBufferToImage(command_buffer.ptr, 
			staging_buffer.ptr, Vk::window.images[Vk::window.image_index],
			Vk::window.extent.width, Vk::window.extent.height
		);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
	}
	command_buffer.end();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Core& core = *(Core *)glfwGetWindowUserPointer(Vk::window.ptr);

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_TAB:
			core.rendering_mode = !core.rendering_mode;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
	}
}

#include <omp.h>

#define BEGIN_LOOP _Pragma("omp parallel for collapse(2)") for (int y = 0; y < height; y++) { for (int x = 0; x < width; x++) { uint32_t index = y * width + x;
#define END_LOOP }}

class ray {
public:
	ray() {}

	ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

	const glm::vec3& origin() const { return orig; }
	const glm::vec3& direction() const { return dir; }

	glm::vec3 at(float t) const {
		return orig + t * dir;
	}

private:
	glm::vec3 orig;
	glm::vec3 dir;
};

struct pixel_s {
	unsigned char b = 0;
	unsigned char g = 0;
	unsigned char r = 0;
	unsigned char a = 255;
};

void Core::cpu_raytracing(void)
{
	int width = (int)Vk::window.extent.width;
	int height = (int)Vk::window.extent.height;
	pixel_s* screen = (pixel_s *)staging_buffer.data;

	BEGIN_LOOP(width, height);

		screen[index].r = char((float)x / width * 255);
		screen[index].g = char((float)y / height * 255);

	END_LOOP;
}