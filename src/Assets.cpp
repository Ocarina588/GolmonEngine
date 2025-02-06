#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GolmonEngine.hpp"


std::unordered_map<std::string, ge::Mesh::ptr> ge::Assets::meshes;
std::vector<ge::Assets::material> ge::Assets::materials;
std::vector<tinygltf::Model> ge::Assets::to_be_loaded;

ge::Sampler::Sampler(void)
{
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.magFilter = VK_FILTER_LINEAR;
	create_info.minFilter = VK_FILTER_LINEAR;
	create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	create_info.mipLodBias = 0.0f;
	create_info.anisotropyEnable = VK_FALSE;
	create_info.maxAnisotropy = 0.f;
	create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	create_info.unnormalizedCoordinates = VK_FALSE;
	create_info.compareEnable = VK_FALSE;
	create_info.compareOp = VK_COMPARE_OP_ALWAYS;
	create_info.minLod = 0.0f;
	create_info.maxLod = VK_LOD_CLAMP_NONE;
}

ge::Sampler::~Sampler(void)
{
	vkDestroySampler(ge::ctx::device, ptr, nullptr);
}

void ge::Sampler::init(void)
{
	if (vkCreateSampler(ge::ctx::device, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture sampler!");;
}

ge::Mesh::Mesh(void)
{

}

ge::Mesh::~Mesh(void)
{

}

void ge::Assets::clear(void)
{
	meshes.clear();
	materials.clear();
	to_be_loaded.clear();
}

void ge::Assets::load_glb(char const* file)
{
	tinygltf::TinyGLTF loader; 
	std::string err;
	std::string warn;

	tinygltf::Model model; 
	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file);
	if (!warn.empty())
		printf("Warn: %s\n", warn.c_str());
	if (!err.empty())
		printf("Err: %s\n", err.c_str());
	if (!ret) {
		printf("Failed to parse glTF\n");
		return;
	}

	if (model.scenes.empty()) {
		std::cerr << "No scene found in the GLB file." << std::endl;
		return;
	}

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	if (scene.nodes.empty()) {
		std::cerr << "No nodes found in the scene." << std::endl;
		return;
	}

	const tinygltf::Node& node = model.nodes[scene.nodes[0]];
	if (node.mesh < 0) {
		std::cerr << "First node does not contain a mesh." << std::endl;
		return;
	}

	const tinygltf::Mesh& mesh = model.meshes[node.mesh];
	Mesh::ptr m = std::make_shared<Mesh>();

	for (const tinygltf::Primitive& primitive : mesh.primitives) {
		load_vertex_data(m, model, primitive);
		load_index_data(m, model, primitive);
	}

	m->material_id = (uint32_t)to_be_loaded.size();
	to_be_loaded.emplace_back(std::move(model));
	meshes[file] = m;
}

void ge::Assets::load_vertex_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
		std::cerr << "Primitive does not contain vertex positions." << std::endl;
		return;
	}

	// Load POSITION data
	int posAccessorIndex = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& posAccessor = model.accessors[posAccessorIndex];
	const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

	const unsigned char* posData = posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset;
	size_t numVertices = posAccessor.count;
	size_t posStride = (posAccessor.ByteStride(posBufferView) > 0) ? posAccessor.ByteStride(posBufferView) : sizeof(float) * 3;

	bool hasUVs = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
	const unsigned char* uvData = nullptr;
	size_t uvStride = 0;

	if (hasUVs) {
		int uvAccessorIndex = primitive.attributes.at("TEXCOORD_0");
		const tinygltf::Accessor& uvAccessor = model.accessors[uvAccessorIndex];
		const tinygltf::BufferView& uvBufferView = model.bufferViews[uvAccessor.bufferView];
		const tinygltf::Buffer& uvBuffer = model.buffers[uvBufferView.buffer];

		uvData = uvBuffer.data.data() + uvBufferView.byteOffset + uvAccessor.byteOffset;
		uvStride = (uvAccessor.ByteStride(uvBufferView) > 0) ? uvAccessor.ByteStride(uvBufferView) : sizeof(float) * 2;
	}

	std::vector<float> vertices;
	for (size_t i = 0; i < numVertices; ++i) {
		float x = *reinterpret_cast<const float*>(posData + i * posStride);
		float y = *reinterpret_cast<const float*>(posData + i * posStride + sizeof(float));
		float z = *reinterpret_cast<const float*>(posData + i * posStride + 2 * sizeof(float));

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);

		if (hasUVs) {
			float u = *reinterpret_cast<const float*>(uvData + i * uvStride);
			float v = *reinterpret_cast<const float*>(uvData + i * uvStride + sizeof(float));

			vertices.push_back(u);
			vertices.push_back(v);
		}
		else {
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
		}
	}

	m->vertex_data.init(
		vertices.data(), (uint32_t)(sizeof(float) * vertices.size()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void ge::Assets::load_index_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive)
{
	if (primitive.indices < 0) {
		std::cerr << "Primitive does not contain indices." << std::endl;
		return;
	}

	const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
	const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
	const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

	const unsigned char* indexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;

	std::vector<uint32_t> indices;
	size_t numIndices = indexAccessor.count;

	for (size_t i = 0; i < numIndices; ++i) {
		uint32_t index;
		if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			index = *reinterpret_cast<const uint16_t*>(indexData + i * sizeof(uint16_t));
		}
		else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
			index = *reinterpret_cast<const uint32_t*>(indexData + i * sizeof(uint32_t));
		}
		else {
			std::cerr << "Unsupported index component type." << std::endl;
			continue;
		}
		indices.push_back(index);
	}

	m->index_data.init(
		(void *)indices.data(), (uint32_t)(sizeof(uint32_t) * indices.size()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}


void ge::Assets::init_materials(ge::CommandBuffer &co)
{
	materials.resize(to_be_loaded.size());

	for (int i = 0; i < to_be_loaded.size(); i++) {
		auto& model = to_be_loaded[i];
		for (const auto& material : model.materials) {
			if (material.normalTexture.index >= 0) {
				int textureIndex = material.normalTexture.index;
				const tinygltf::Texture& texture = model.textures[textureIndex];

				if (texture.source >= 0) {  // Get the corresponding image
					const tinygltf::Image& image = model.images[texture.source];

					std::cout << "Normal Map found: " << image.uri << std::endl;
					std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
					std::cout << "Component Count: " << image.component << std::endl; // 3 (RGB) or 4 (RGBA)

					// Access raw pixel data
					const unsigned char* imageData = image.image.data();
					size_t imageSize = image.image.size();  // Total bytes

					if (!imageData || imageSize == 0) {
						std::cerr << "Error: Normal map image data is empty!" << std::endl;
						continue;
					}

					// Upload the texture to Vulkan
					materials[i].normal.init_raw(
						co,
						image.image.data(), image.width, image.height, image.image.size(),  // Corrected width calculation
						VK_FORMAT_R8G8B8A8_SRGB,
						VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
						VK_IMAGE_ASPECT_COLOR_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
					);
				}
			}

			if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) { // Check if a base color texture exists
				int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
				const tinygltf::Texture& texture = model.textures[textureIndex];

				if (texture.source >= 0) { // Retrieve the image associated with this texture
					const tinygltf::Image& image = model.images[texture.source];

					std::cout << "Color Texture found: " << image.uri << std::endl;
					std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
					std::cout << "Component Count: " << image.component << std::endl; // Usually 3 (RGB) or 4 (RGBA)

					// Access the raw image data

					if (!image.image.data() || image.image.size() == 0) {
						std::cerr << "Error: Color map image data is empty!" << std::endl;
						continue;
					}

					materials[i].color.init_raw(
						co,
						image.image.data(), image.width, image.height, image.image.size(),  // Corrected width calculation
						VK_FORMAT_R8G8B8A8_SRGB,
						VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
						VK_IMAGE_ASPECT_COLOR_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
					);
					std::cout << "nice" << std::endl;
				}
			}
		}
	}
}

//void ge::Mesh::load_textures(tinygltf::Model const& model)
//{
//	for (const auto& material : model.materials) {
//		if (material.normalTexture.index >= 0) {
//			int textureIndex = material.normalTexture.index;
//			const tinygltf::Texture& texture = model.textures[textureIndex];
//
//			if (texture.source >= 0) {  // Get the corresponding image
//				const tinygltf::Image& image = model.images[texture.source];
//
//				std::cout << "Normal Map found: " << image.uri << std::endl;
//				std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
//				std::cout << "Component Count: " << image.component << std::endl; // 3 (RGB) or 4 (RGBA)
//
//				// Access raw pixel data
//				const unsigned char* imageData = image.image.data();
//				size_t imageSize = image.image.size();  // Total bytes
//
//				if (!imageData || imageSize == 0) {
//					std::cerr << "Error: Normal map image data is empty!" << std::endl;
//					continue;
//				}
//
//				// Upload the texture to Vulkan
//				//normal.init_compressed(
//				//	co,
//				//	imageData, image.width * image.component,  // Corrected width calculation
//				//	VK_FORMAT_R8G8B8A8_SRGB,
//				//	VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//				//	VK_IMAGE_ASPECT_COLOR_BIT,
//				//	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
//				//);
//			}
//		}
//
//		if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) { // Check if a base color texture exists
//			int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
//			const tinygltf::Texture& texture = model.textures[textureIndex];
//
//			if (texture.source >= 0) { // Retrieve the image associated with this texture
//				const tinygltf::Image& image = model.images[texture.source];
//
//				std::cout << "Color Texture found: " << image.uri << std::endl;
//				std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
//				std::cout << "Component Count: " << image.component << std::endl; // Usually 3 (RGB) or 4 (RGBA)
//
//				// Access the raw image data
//				const std::vector<unsigned char>& imageData = image.image;
//			}
//		}
//	}
//
//}

void ge::Mesh::draw(ge::CommandBuffer& command_buffer)
{
	//std::cout << vertex_data.size() << " " << index_data.size() << std::endl;
	VkDeviceSize offset = 0;
	if (index_data.ptr == nullptr) {
		//for (int i = 0; i < vertex_data.size(); i++) {
			vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_data.ptr, &offset);
			vkCmdDraw(command_buffer.ptr, (uint32_t)vertex_data.original_size / sizeof(glm::vec3), 1, 0, 0);
		//}
		return;
	}

	//for (int i = 0; i < vertex_data.size(); i++) {
		vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_data.ptr, &offset);
		vkCmdBindIndexBuffer(command_buffer.ptr, index_data.ptr, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(command_buffer.ptr, (uint32_t)index_data.original_size / sizeof(uint32_t), 1, 0, 0, 0);
	//}

}

stbi_uc* ge::Assets::load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
}
