#include "../../include/UI/UI.hpp"
#include "../imgui/imconfig.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_vulkan.h"
#include "Core.hpp"
#include "Windows.h"

ge::UI::UI(void)
{

}

ge::UI::~UI(void)
{
	vkDestroyDescriptorPool(ge::ctx::device, imguiPool, nullptr);
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


void ge::UI::init(VkRenderPass render_pass) {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(ge::ctx::window, true);

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
	if (vkCreateDescriptorPool(ge::ctx::device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = ge::ctx::instance;
	init_info.PhysicalDevice = ge::ctx::device;
	init_info.Device = ge::ctx::device;
	init_info.Queue = ge::ctx::device.queue.graphics;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = ge::ctx::window.capabilites.minImageCount;
	init_info.ImageCount = ge::ctx::window.capabilites.minImageCount + 1;
	init_info.RenderPass = render_pass;
	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();
}

void ge::UI::render(VkCommandBuffer cmd)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();


	if (ImGui::BeginMainMenuBar()) {


		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open file")) ask_file("");;// file_to_load = ask_file("").c_str();
			ImGui::EndMenu();
		}

		//ImGui::ShowDemoWindow();

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
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}