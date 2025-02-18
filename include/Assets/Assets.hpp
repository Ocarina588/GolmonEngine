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
		glm::vec3 tangent; 
		glm::vec3 bitangent;
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
			std::vector<VkVertexInputAttributeDescription> attributes(5);

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
			attributes[2].offset = offsetof(Vertex, tangent);
			attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributes[3].binding = 0;
			attributes[3].location = 3;
			attributes[3].offset = offsetof(Vertex, bitangent);
			attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributes[4].binding = 0;
			attributes[4].location = 4;
			attributes[4].offset = offsetof(Vertex, uv);
			attributes[4].format = VK_FORMAT_R32G32_SFLOAT;

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

		uint32_t material_id;
		void draw(ge::CommandBuffer& command_buffer);
		void load_vertices(std::vector<Vertex> const& vertices);
		void load_indices(std::vector<unsigned int> const& indices);
	private:
		ge::Buffer vertex_data;
		ge::Buffer index_data;
	};

	class Assets {
	public:

		Assets(void);
		~Assets(void);
		
		static void clear(void);

		struct material_s {
			uint32_t index_albedo = 42; 
			uint32_t index_normal = 42; 
			uint32_t index_metallic = 42; 
			uint32_t index_roughness = 42;
			uint32_t index_emissive = 42; 
			uint32_t index_occlusion = 42; 
			uint32_t index_debug = 42;
			uint32_t light_equation = 42;
		};

		static void load_assimp(char const* file);
		static void upload_textures(ge::CommandBuffer &co);
		static void load_model(char const* file);

		static std::vector<Mesh::ptr> meshes;
		static std::vector<material_s> materials;
		static std::vector<ge::Image> textures;

		static uint8_t* load_from_memory(uint8_t const* buffer, int len, int* x, int* y, int* comp, int req_comp);
		static uint8_t* load_from_file(char const* file, int* x, int* y, int* comp, int req_comp);
		static float* read_filef(char const* file, int& w, int& h, int& c, int expected);
	private:

		struct texture_s {
			int w = 0, h = 0, c = 0;
			uint8_t* data = nullptr;
		};
		
		static std::vector<texture_s> to_be_loaded; 
		
		static void load_assimp_vertices(void const * scene);
		static void load_assimp_materials(void const * scene);

		static void compute_tangent(Vertex* p, std::vector<uint32_t> const& indices);
		static void load_gltf_texture(tinygltf::Model const& model, ge::CommandBuffer& co, ge::Image& m, int id);
		static std::vector<float> load_vertex_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive);
		static std::vector<uint32_t> load_index_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive);
	};

}