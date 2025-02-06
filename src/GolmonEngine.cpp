#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GolmonEngine.hpp"
#include "Context/Context.hpp"

ge::Instance ge::ctx::instance;
ge::Device ge::ctx::device;
ge::Window ge::ctx::window;

using Vk = ge::ctx;

void ge::ctx::use_debug(void)
{
#ifdef _DEBUG
	Vk::instance._debug = true;
#endif
}

void ge::ctx::set_extent(uint32_t w, uint32_t h)
{
	Vk::device.extent = { w, h };
}

void ge::ctx::set_mode(VkPresentModeKHR mode)
{
	Vk::window.mode = mode;
}

void ge::ctx::use_window(char const* t)
{
	Vk::window.init(static_cast<int>(Vk::device.extent.width), static_cast<int>(Vk::device.extent.height), t);
}

void ge::ctx::add_layer(char const* l)
{
	Vk::instance._layers.push_back(l);
}

void ge::ctx::add_instance_extension(char const* e)
{
	Vk::instance._extensions.push_back(e);
}

void ge::ctx::add_device_extension(char const* e)
{
	Vk::device._extensions.push_back(e);
}

void ge::ctx::use_gpu(uint32_t index)
{
	Vk::device._gpu = index;
}

void ge::ctx::init(void)
{
	if (window.ptr) {
		for (auto e : window.get_required_extensions())
			instance._extensions.push_back(e);
		device._extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	if (instance._debug) {
		instance._extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance._layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	instance.init();

	if (window.ptr)
		window.init_surface();

	device.init();

	if (window.ptr)
		window.init_swapchain();
}

ge::Mesh::Mesh(void)
{

}

ge::Mesh::~Mesh(void)
{

}

void ge::Mesh::load_from_glb(char const* file)
{
	tinygltf::Model model; 
	tinygltf::TinyGLTF loader; 
	std::string err;
	std::string warn;

	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file);
	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

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

	for (const tinygltf::Primitive& primitive : mesh.primitives) {
		load_vertex_data(model, primitive);
		load_index_data(model, primitive);
	}

}

void ge::Mesh::load_vertex_data(tinygltf::Model const& model, tinygltf::Primitive const& primitive)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
		std::cerr << "Primitive does not contain vertex positions." << std::endl;
		return;
	}

	int posAccessorIndex = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& posAccessor = model.accessors[posAccessorIndex];
	const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

	const unsigned char* posData = posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset;

	std::vector<float> vertices;
	size_t numVertices = posAccessor.count;
	size_t stride = (posAccessor.ByteStride(posBufferView) > 0) ? posAccessor.ByteStride(posBufferView) : sizeof(float) * 3;

	for (size_t i = 0; i < numVertices; ++i) {
		float x = *reinterpret_cast<const float*>(posData + i * stride);
		float y = *reinterpret_cast<const float*>(posData + i * stride + sizeof(float));
		float z = *reinterpret_cast<const float*>(posData + i * stride + 2 * sizeof(float));

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
	}

	vertex_data.emplace_back();
	vertex_data.rbegin()->init(
		vertices.data(), (uint32_t)(sizeof(float) * vertices.size()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void ge::Mesh::load_index_data(tinygltf::Model const& model, tinygltf::Primitive const& primitive)
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

	index_data.emplace_back();
	index_data.rbegin()->init(
		(void *)indices.data(), (uint32_t)(sizeof(uint32_t) * indices.size()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void ge::Mesh::draw(ge::CommandBuffer& command_buffer)
{
	VkDeviceSize offset = 0;
	if (index_data.empty()) {
		for (int i = 0; i < vertex_data.size(); i++) {
			vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_data[i].ptr, &offset);
			vkCmdDraw(command_buffer.ptr, (uint32_t)vertex_data[i].original_size / sizeof(glm::vec3), 1, 0, 0);
		}
		return;
	}

	for (int i = 0; i < vertex_data.size(); i++) {
		vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_data[i].ptr, &offset);
		vkCmdBindIndexBuffer(command_buffer.ptr, index_data[i].ptr, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(command_buffer.ptr, (uint32_t)index_data[i].original_size / sizeof(uint32_t), 1, 0, 0, 0);
	}

}