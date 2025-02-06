#pragma once

#include "Context/Context.hpp"

#include "Context/Window.hpp"
#include "Context/Instance.hpp"
#include "Context/Device.hpp"

#include "Objects/Image.hpp"
#include "Objects/Shader.hpp"
#include "Objects/Sync.hpp"
#include "Objects/GraphicsPipeline.hpp"
#include "Objects/Descriptor.hpp"
#include "Objects/Commands.hpp"
#include "Objects/Buffer.hpp"
#include "Objects/Camera.hpp"

#include "utils.hpp"

#include "Events.hpp"

#include <tiny_gltf.h>
#include <glm/glm.hpp>

namespace ge {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;

		static VkVertexInputBindingDescription get_binding(void)
		{
			VkVertexInputBindingDescription binding{};

			binding.binding = 0;
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding.stride = sizeof(Vertex);

			return binding;
		}
		static std::vector<VkVertexInputAttributeDescription> get_attribute(void)
		{
			std::vector<VkVertexInputAttributeDescription> attributes(2);

			attributes[0].binding = 0;
			attributes[0].location = 0;
			attributes[0].offset = offsetof(Vertex, pos);
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributes[1].binding = 0;
			attributes[1].location = 1;
			attributes[1].offset = offsetof(Vertex, uv);
			attributes[1].format = VK_FORMAT_R32G32_SFLOAT;

			return attributes;
		}
	};

	class Mesh {
	public:
		Mesh(void);
		~Mesh(void);

		void load_from_glb(char const* file);
		void draw(ge::CommandBuffer& command_buffer);
	private:
		std::vector<ge::Buffer> vertex_data;
		std::vector<ge::Buffer> index_data;
		
		ge::Image color;
		ge::Image normal;
		
		void load_vertex_data(tinygltf::Model const &model, tinygltf::Primitive const & primitive);
		void load_index_data(tinygltf::Model const& model, tinygltf::Primitive const& primitive);
		void load_textures(tinygltf::Model const& model);
	};


}