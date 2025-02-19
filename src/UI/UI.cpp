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
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

void ge::UI::render(VkCommandBuffer cmd)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open file"));
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Camera")) {
			if (ImGui::MenuItem("Scene"));
			if (ImGui::MenuItem("Player"));
			ImGui::EndMenu();
		}

	}

	ImGui::EndMainMenuBar();


	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.2f;
	static float sidePanelWidth = 300.0f; // Default width of the panel

	// Position the window at the right side of the screen
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - sidePanelWidth, 18));
	ImGui::SetNextWindowSize(ImVec2(sidePanelWidth, io.DisplaySize.y));
	ImGui::SetNextWindowSizeConstraints(ImVec2(100, io.DisplaySize.y), ImVec2(600, io.DisplaySize.y)); // Min:100px, Max:600px

	ImGui::Begin("Side Panel", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse
	);

	const char* render_pass_item_p[] = { "Combined", "Albedo", "Normal", "Mettalic", "Roughness", "Emissive", "Occlusion"};
	static int currentItem = 0;  // The selected item index
	ImGui::SetNextItemWidth(120);
	if (ImGui::Combo("Render Pass", &currentItem, render_pass_item_p, IM_ARRAYSIZE(render_pass_item_p)))
		render_pass_item = currentItem;
	static int currentItema = 0;  // The selected item index
	const char* light_equation_type_p[] = {"BRDF Disney", "PHONG"};
	ImGui::SetNextItemWidth(120);
	if (ImGui::Combo("Light Equation", &currentItema, light_equation_type_p, IM_ARRAYSIZE(light_equation_type_p)))
		light_equation_type = currentItema;


	ImGui::End();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	for (auto& m : ge::Assets::materials) {
		m.index_debug = render_pass_item;
		m.light_equation = light_equation_type;
	}
}

void ge::UI::init(Core *core) {
	core = core;
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
	init_info.RenderPass = core->render_pass.ptr;
	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();
}


/*
void ge::UI::render(VkCommandBuffer cmd) 
{
	
}
*/