#pragma once

#include <tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "Objects/Commands.hpp"
#include "Objects/Buffer.hpp"

namespace ge {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
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
			std::vector<VkVertexInputAttributeDescription> attributes(3);

			attributes[0].binding = 0;
			attributes[0].location = 0;
			attributes[0].offset = offsetof(Vertex, pos);
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributes[1].binding = 0;
			attributes[1].location = 1;
			attributes[1].offset = offsetof(Vertex, normal);
			attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributes[2].binding = 0;
			attributes[2].location = 2;
			attributes[2].offset = offsetof(Vertex, uv);
			attributes[2].format = VK_FORMAT_R32G32_SFLOAT;

			return attributes;
		}
	};

	class Sampler {
	public:
		Sampler(void);
		~Sampler(void);

		operator VkSampler () { return ptr; };

		void init(void);

		VkSamplerCreateInfo create_info{};

	private:
		VkSampler ptr = nullptr;
	};

	class Mesh {
	public:
		friend class Assets;
		typedef std::shared_ptr<Mesh> ptr;
		
		Mesh(void);
		~Mesh(void);

		void draw(ge::CommandBuffer& command_buffer);
	private:
		ge::Buffer vertex_data;
		ge::Buffer index_data;
		uint32_t material_id;
	};

	class Assets {
	public:
		Assets(void);
		~Assets(void);
		
		static void clear(void);

		struct material {
			ge::Image albedo;
			ge::Image normal;
			ge::Image metallic;
			ge::Image emissive;
			ge::Image occlusion;
		};

		static void load_glb(char const* file);
		static void init_materials(ge::CommandBuffer& co);

		static std::unordered_map<std::string, Mesh::ptr> meshes;
		static std::vector<material> materials;

		static uint8_t* load_from_memory(uint8_t const* buffer, int len, int* x, int* y, int* comp, int req_comp);

	private:
		
		static std::vector<tinygltf::Model> to_be_loaded;

		static void load_gltf_texture(tinygltf::Model const& model, ge::CommandBuffer& co, ge::Image& m, int id);
		static void load_vertex_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive);
		static void load_index_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive);
	};

}