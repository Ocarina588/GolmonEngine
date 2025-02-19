#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include "Objects/Commands.hpp"

namespace ge {

	class Image;
	class CommandBuffer;

	struct PushConstant {
		uint32_t index_albedo;
		uint32_t index_normal;
		uint32_t index_metallic;
		uint32_t index_emissive;
		uint32_t index_occlusion;
	};

	class RenderPass {
	public:
		friend class GraphicsPipeline;
		friend class Image;
		RenderPass(void);
		~RenderPass(void);

		void init(void);
		void init(VkRenderPassCreateInfo i);

		inline void use_depth(Image& _depth_image) { subpass_description.pDepthStencilAttachment = &depth_ref; depth_image = &_depth_image; }
		inline void set_final_layout(VkImageLayout layout) { color_attachment.finalLayout = layout; }
		inline void set_initial_layout(VkImageLayout layout) { color_attachment.initialLayout = layout; }
		inline void set_clear_color(glm::vec3 color) { clear_color = { {{color.x / 255, color.y / 255, color.z / 255}} }; }
		void begin(CommandBuffer& buffer, VkFramebuffer frame_buffer = nullptr, VkExtent2D extent = {});
		void end(CommandBuffer& buffer);

		VkRenderPass ptr = nullptr;
		VkRenderPassCreateInfo create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	private:
		VkAttachmentDescription color_attachment{
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_GENERAL,
			.finalLayout = VK_IMAGE_LAYOUT_GENERAL,
		};
		VkAttachmentDescription depth_attachment{};
		VkAttachmentReference color_ref{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depth_ref{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass_description{};

		VkClearValue clear_color = { {{0.f, 0.f, 0.f}} };
		Image* depth_image = nullptr;
	};

	class Pipeline {
	public:
		Pipeline(void);
		~Pipeline(void);

		template<class T> void add_push_constant(VkShaderStageFlags stage) {
			VkPushConstantRange range{};
			range.stageFlags = stage;
			range.size = sizeof(T);
			push_constants.push_back(range);
		}
		inline void add_shader_stage(VkPipelineShaderStageCreateInfo i) { stages.push_back(i); }
		inline void add_binding(VkVertexInputBindingDescription i) { bindings.push_back(i); }
		inline void add_attribute(VkVertexInputAttributeDescription i) { attributes.push_back(i); }
		inline void add_layout(VkDescriptorSetLayout i) { layouts.push_back(i); }
		virtual void init(void) = 0;

		inline void bind(CommandBuffer& command_buffer) { vkCmdBindPipeline(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, ptr); }

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkPushConstantRange> push_constants;
		VkPipelineLayout layout = nullptr;
		VkPipeline ptr = nullptr;
	};

	class GraphicsPipeline : public Pipeline {
	public:
		GraphicsPipeline(void);
		~GraphicsPipeline(void);

		virtual void init(void);

		inline void set_render_pass(RenderPass& _render_pass) {
			render_pass = _render_pass.ptr;
			if (_render_pass.subpass_description.pDepthStencilAttachment == nullptr) return;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		}

		VkRenderPass render_pass = nullptr;
		VkPipelineVertexInputStateCreateInfo vertex_input_state{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineInputAssemblyStateCreateInfo input_assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		VkPipelineRasterizationStateCreateInfo rasterizer{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.lineWidth = 1.f,
		};
		VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		VkPipelineMultisampleStateCreateInfo multisample{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		VkPipelineViewportStateCreateInfo viewport_state{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		VkViewport viewport{};
		VkRect2D scissors{};
		VkPipelineColorBlendAttachmentState attachment{};
		VkPipelineColorBlendStateCreateInfo color_blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	};

	class ComputePipeline : public Pipeline {
		public:
			ComputePipeline(void);
			~ComputePipeline(void);

			virtual void init(void);

		private:

	};
}