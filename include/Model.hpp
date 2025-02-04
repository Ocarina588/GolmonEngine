#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <array>
#include <assimp/Importer.hpp>      // Assimp importer class
#include <assimp/scene.h>  
#include <assimp/postprocess.h>// Main data structure for scenes
#include "utils.hpp"
#include "Context/Context.hpp"
#include "Objects/Buffer.hpp"
#include "Objects/Commands.hpp"
#include "Objects/Image.hpp"
#include "objloader.hpp"



struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 normal;
	alignas(16) glm::vec3 text_coord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, text_coord);

		return attributeDescriptions;
	}
};

struct material_s {
	std::string name;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emissive;
	float smooth = 0.f;
	float specular_probability = 0.f;
};

struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Ray {
public:
	Ray() {}

	Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

	glm::vec3 at(float t) const {
		return orig + t * dir;
	}

	glm::vec3 orig;
	glm::vec3 dir;
};

struct hit_info {
	float distance = 100000.f;
	int material_index;
	glm::vec3 pos;
	glm::vec3 normal;
};

class Mesh {
public:
	Mesh(void);
	~Mesh(void);

	void init(void* data, size_t size);
	void init(std::vector<Vertex>& _vertices);

	bool hit(Ray& r, hit_info &info);
	vulkan::Buffer buffer;
	std::vector<Vertex> vertices;
	std::string name;
	int material_index;
	int texture_index;

private:
};

class Model {
public:
	Model(void);
	~Model(void);
private:
};

class Scene {
public:
	Scene(void);
	~Scene(void);

	void draw(vulkan::CommandBuffer& b);

	void load_obj(char const* file_name);
	void load_scene(std::string const& file_name, vulkan::CommandBuffer &co);
	std::vector<Mesh> meshes;
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
	std::vector<material_s> materials;
	std::vector<vulkan::Image> textures;
	uint32_t total_size = 0;
private:
};