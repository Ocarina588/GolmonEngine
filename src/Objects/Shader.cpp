#include "GolmonEngine.hpp"

ge::Shader::Shader(char const* file_name, VkShaderStageFlagBits flag)
{
	auto data = utils::readFile(file_name);

	VkShaderModuleCreateInfo create_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	create_info.pCode = reinterpret_cast<uint32_t*>(data.data());
	create_info.codeSize = static_cast<uint32_t>(data.size());

	if (vkCreateShaderModule(ctx::device.ptr, &create_info, nullptr, &stage.module) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module");

	stage.pName = "main";
	stage.stage = flag;
}

ge::Shader::~Shader(void)
{
	vkDestroyShaderModule(ctx::device.ptr, stage.module, nullptr);
}