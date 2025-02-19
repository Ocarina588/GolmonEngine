#include "GolmonEngine.hpp"

using namespace ge;

RenderPass::RenderPass(void)
{

}

RenderPass::~RenderPass(void)
{
	vkDestroyRenderPass(ctx::device.ptr, ptr, nullptr);
}

void RenderPass::init(VkRenderPassCreateInfo i)
{
	if (vkCreateRenderPass(ctx::device.ptr, &i, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create render pass");
}

void RenderPass::init(void)
{
	VkRenderPassCreateInfo create_info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

	color_attachment.format = ctx::device.format.format;

	if (depth_image) {
		depth_attachment.format = depth_image->format;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_ref;
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription tab[] = { color_attachment, depth_attachment };
	create_info.attachmentCount = 1 + (subpass_description.pDepthStencilAttachment != nullptr);
	create_info.pAttachments = tab;
	create_info.subpassCount = 1;
	create_info.pSubpasses = &subpass_description;
	create_info.dependencyCount = 1;
	create_info.pDependencies = &dependency;

	if (vkCreateRenderPass(ctx::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create render pass");
}

void RenderPass::begin(CommandBuffer& buffer, VkFramebuffer frame_buffer, VkExtent2D extent)
{
	VkClearValue clear_depth = { .depthStencil = {1.f, 0} };
	VkClearValue clear_values[] = { clear_color, clear_depth };

	VkRenderPassBeginInfo begin_info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	begin_info.clearValueCount = 1 + (depth_image != nullptr);
	begin_info.pClearValues = clear_values;
	if (frame_buffer)
		begin_info.framebuffer = frame_buffer;
	else
		begin_info.framebuffer = ge::ctx::window.images[ge::ctx::window.image_index];
	begin_info.renderArea.offset = {};
	if (extent.width == 0 && extent.height == 0)
		begin_info.renderArea.extent = ge::ctx::device.extent;
	else
		begin_info.renderArea.extent = extent;
	begin_info.renderPass = ptr;

	vkCmdBeginRenderPass(buffer.ptr, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(CommandBuffer& buffer)
{
	vkCmdEndRenderPass(buffer.ptr);
}

Pipeline::Pipeline(void)
{

}

Pipeline::~Pipeline(void)
{

}

GraphicsPipeline::GraphicsPipeline(void)
{

}

GraphicsPipeline::~GraphicsPipeline(void)
{
	vkDestroyPipelineLayout(ctx::device.ptr, layout, nullptr);
	vkDestroyPipeline(ctx::device.ptr, ptr, nullptr);
}

void GraphicsPipeline::init(void)
{
	VkGraphicsPipelineCreateInfo create_info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };


	{ // VERTEX INPUT
		vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
		vertex_input_state.pVertexAttributeDescriptions = attributes.data();
		vertex_input_state.pVertexBindingDescriptions = bindings.data();
	}

	{ // INPUT ASSEMBLY
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	{ //VIEWPORT STATE
		viewport = { 0, 0, static_cast<float>(ctx::device.extent.width), static_cast<float>(ctx::device.extent.height), 0.f, 1.f };
		scissors = { {0, 0}, ctx::device.extent.width, ctx::device.extent.height };
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissors;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
	}

	{ //RATERIZER
		//rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//rasterizer.cullMode = VK_CULL_MODE_NONE;
		//rasterizer.lineWidth = 1.f;
		//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	{ // MULTISAMPLE
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	{ // COLOR BLEND ATTACHMENT
		attachment.colorWriteMask = 0b1111;
		color_blend.attachmentCount = 1;
		color_blend.pAttachments = &attachment;
	}

	VkPipelineLayoutCreateInfo layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	layout_create_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
	layout_create_info.pSetLayouts = layouts.data();
	layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(push_constants.size());
	layout_create_info.pPushConstantRanges = push_constants.data();

	if (vkCreatePipelineLayout(ctx::device.ptr, &layout_create_info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");

	create_info.stageCount = static_cast<uint32_t>(stages.size());
	create_info.pStages = stages.data();
	create_info.pVertexInputState = &vertex_input_state;
	create_info.pInputAssemblyState = &input_assembly;
	create_info.pViewportState = &viewport_state;
	create_info.pRasterizationState = &rasterizer;
	create_info.pMultisampleState = &multisample;
	create_info.pColorBlendState = &color_blend;
	create_info.pDepthStencilState = &depthStencil;
	create_info.layout = layout;
	create_info.renderPass = render_pass;

	if (vkCreateGraphicsPipelines(ctx::device.ptr, nullptr, 1, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline");
}

ge::ComputePipeline::ComputePipeline(void)
{

}

ge::ComputePipeline::~ComputePipeline(void)
{

}

void ge::ComputePipeline::init(void)
{
	VkComputePipelineCreateInfo create_info{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	create_info.stage = stages[0];

	VkPipelineLayoutCreateInfo layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	layout_create_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
	layout_create_info.pSetLayouts = layouts.data();
	layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(push_constants.size());
	layout_create_info.pPushConstantRanges = push_constants.data();

	if (vkCreatePipelineLayout(ctx::device.ptr, &layout_create_info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");

	if (vkCreateComputePipelines(ctx::device.ptr, nullptr, 1, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create compute pipeline");
}