#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GolmonEngine.hpp"

// MIKKSTPACE

#include "Assets/mikktspace.h"

struct mikktspace_data {
	ge::Vertex* vertices = nullptr;
	uint32_t* indices = nullptr;
	size_t size = 0;
};


// MikkTSpace callbacks
static int getNumFaces(const SMikkTSpaceContext* pContext) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	return mesh->size / 3; // Assuming a triangle mesh
}

static int getNumVerticesOfFace(const SMikkTSpaceContext* pContext, int iFace) {
	return 3; // glTF uses triangles
}

static void getPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], int iFace, int iVert) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	uint32_t index = mesh->indices[iFace * 3 + iVert];
	glm::vec3 pos = mesh->vertices[index].pos;
	fvPosOut[0] = pos.x;
	fvPosOut[1] = pos.y;
	fvPosOut[2] = pos.z;
}

static void getNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], int iFace, int iVert) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	uint32_t index = mesh->indices[iFace * 3 + iVert];
	glm::vec3 norm = mesh->vertices[index].normal;
	fvNormOut[0] = norm.x;
	fvNormOut[1] = norm.y;
	fvNormOut[2] = norm.z;
}

static void getTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], int iFace, int iVert) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	uint32_t index = mesh->indices[iFace * 3 + iVert];
	glm::vec2 uv = mesh->vertices[index].uv;
	fvTexcOut[0] = uv.x;
	fvTexcOut[1] = uv.y;
}

static void setTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], float fSign, int iFace, int iVert) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	uint32_t index = mesh->indices[iFace * 3 + iVert];
	mesh->vertices[index].tangent = glm::vec3(fvTangent[0], fvTangent[1], fvTangent[2]);
	mesh->vertices[index].bitangent = glm::normalize(glm::cross(mesh->vertices[index].normal, glm::vec3(mesh->vertices[index].tangent)) * fSign);
}

void generateTangents(mikktspace_data& mesh) {

	SMikkTSpaceInterface iface = {};
	iface.m_getNumFaces = getNumFaces;
	iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
	iface.m_getPosition = getPosition;
	iface.m_getNormal = getNormal;
	iface.m_getTexCoord = getTexCoord;
	iface.m_setTSpaceBasic = setTSpaceBasic;

	SMikkTSpaceContext context = {};
	context.m_pInterface = &iface;
	context.m_pUserData = &mesh;

	genTangSpaceDefault(&context); // Run the algorithm
}

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
	std::cout << "nodes : " << model.nodes.size() << std::endl;
	const tinygltf::Node& node = model.nodes[scene.nodes[0]];
	if (node.mesh < 0) {
		std::cerr << "First node does not contain a mesh." << std::endl;
		return;
	}

	const tinygltf::Mesh& mesh = model.meshes[node.mesh];
	Mesh::ptr m = std::make_shared<Mesh>();
	
	std::cout << "primitives: " << mesh.primitives.size() << std::endl;

	for (const tinygltf::Primitive& primitive : mesh.primitives) {
		auto v_data = load_vertex_data(m, model, primitive);
		auto i_data = load_index_data(m, model, primitive);

		std::cout << i_data.size() << " " << v_data.size() << " " << v_data.size() / 11 << std::endl;
		if (primitive.attributes.find("TANGENT") == primitive.attributes.end()) {
			std::cout << "No tangent data found, computing it myself then" << std::endl;
			mikktspace_data tmp;
			tmp.indices = i_data.data();
			tmp.vertices = reinterpret_cast<ge::Vertex*>(v_data.data());
			tmp.size = i_data.size();
			generateTangents(tmp);
			//compute_tangent(reinterpret_cast<Vertex*>(v_data.data()), i_data);
		}

		m->vertex_data.init(
			v_data.data(), (uint32_t)(sizeof(float) * v_data.size()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		m->index_data.init(
			(void*)i_data.data(), (uint32_t)(sizeof(uint32_t) * i_data.size()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	}

	m->material_id = (uint32_t)to_be_loaded.size();
	to_be_loaded.emplace_back(std::move(model));
	meshes[file] = m;
}

std::vector<float> ge::Assets::load_vertex_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
		std::cerr << "Primitive does not contain vertex positions." << std::endl;
		return {};
	}

	// Load POSITION data
	int posAccessorIndex = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& posAccessor = model.accessors[posAccessorIndex];
	const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

	const unsigned char* posData = posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset;
	size_t numVertices = posAccessor.count;
	size_t posStride = (posAccessor.ByteStride(posBufferView) > 0) ? posAccessor.ByteStride(posBufferView) : sizeof(float) * 3;

	// Check if normals exist
	bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
	const unsigned char* normalData = nullptr;
	size_t normalStride = 0;

	if (hasNormals) {
		int normalAccessorIndex = primitive.attributes.at("NORMAL");
		const tinygltf::Accessor& normalAccessor = model.accessors[normalAccessorIndex];
		const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
		const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];

		normalData = normalBuffer.data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset;
		normalStride = (normalAccessor.ByteStride(normalBufferView) > 0) ? normalAccessor.ByteStride(normalBufferView) : sizeof(float) * 3;
	}

	// Check if Tangent exist
	bool hasTangent = primitive.attributes.find("TANGENT") != primitive.attributes.end();
	const unsigned char* tangentData = nullptr;
	size_t tangentStride = 0;

	if (hasTangent) {
		int tangentAccessorIndex = primitive.attributes.at("TANGENT");
		const tinygltf::Accessor& tangentAccessor = model.accessors[tangentAccessorIndex];
		const tinygltf::BufferView& tangentBufferView = model.bufferViews[tangentAccessor.bufferView];
		const tinygltf::Buffer& tangentBuffer = model.buffers[tangentBufferView.buffer];

		tangentData = tangentBuffer.data.data() + tangentBufferView.byteOffset + tangentAccessor.byteOffset;
		tangentStride = (tangentAccessor.ByteStride(tangentBufferView) > 0) ? tangentAccessor.ByteStride(tangentBufferView) : sizeof(float) * 3;
	}

	// Check if UVs exist
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
		// Read Position
		float x = *reinterpret_cast<const float*>(posData + i * posStride);
		float y = *reinterpret_cast<const float*>(posData + i * posStride + sizeof(float));
		float z = *reinterpret_cast<const float*>(posData + i * posStride + 2 * sizeof(float));

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);

		// Read Normal (if available, otherwise set to (0,0,1))
		if (hasNormals) {
			float nx = *reinterpret_cast<const float*>(normalData + i * normalStride);
			float ny = *reinterpret_cast<const float*>(normalData + i * normalStride + sizeof(float));
			float nz = *reinterpret_cast<const float*>(normalData + i * normalStride + 2 * sizeof(float)); 

			vertices.push_back(nx);
			vertices.push_back(ny);
			vertices.push_back(nz); 
		}
		else {
			vertices.push_back(0.0f); // Default normal (0,0,1)
			vertices.push_back(0.0f); 
			vertices.push_back(0.0f);
		}
		//TANGENT
		if (hasTangent) {
			float nx = *reinterpret_cast<const float*>(tangentData + i * tangentStride);
			float ny = *reinterpret_cast<const float*>(tangentData + i * tangentStride + sizeof(float));
			float nz = *reinterpret_cast<const float*>(tangentData + i * tangentStride + 2 * sizeof(float));

			vertices.push_back(nx);
			vertices.push_back(ny);
			vertices.push_back(nz); 
		}
		else {
			vertices.push_back(0.0f); // Default normal (0,0,1)
			vertices.push_back(0.0f); 
			vertices.push_back(1.0f);
		}

		//BITANGENT
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
		// Read UV (if available, otherwise set to (0,0))
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

	return vertices;
}

void ge::Assets::compute_tangent(Vertex *p, std::vector<uint32_t> const &indices)
{
	for (int i = 0; i < indices.size(); i+=3) {

		//my version
		/*
		Vertex& A = p[indices[i]];
		Vertex& B = p[indices[i + 1]];
		Vertex& C = p[indices[i + 2]];

		glm::vec3 E1 = B.pos - A.pos;
		glm::vec3 E2 = C.pos - A.pos;
		glm::vec2 L1 = B.uv - A.uv;
		glm::vec2 L2 = C.uv - A.uv;

		glm::mat2 inv = glm::inverse(glm::mat2(L1.x, L1.y, L2.x, L2.y));
		
		glm::vec3 tangent;

		tangent.x = inv[0].x * E1.x + inv[0].y * E2.x;
		tangent.y = inv[0].x * E1.y + inv[0].y * E2.y;
		tangent.z = inv[0].x * E1.z + inv[0].y * E2.z;

		// Store them
		A.tangent += tangent;
		B.tangent += tangent;
		C.tangent += tangent; 
		*/
		//OGLDEV VERSION

	//	/*

		/*
		Vertex& v0 = p[indices[i]];
		Vertex& v1 = p[indices[i + 1]];
		Vertex& v2 = p[indices[i + 2]];

		glm::vec3 Edge1 = v1.pos - v0.pos;
		glm::vec3 Edge2 = v2.pos - v0.pos;

		float DeltaU1 = v1.uv.x - v0.uv.x;
		float DeltaV1 = v1.uv.y - v0.uv.y;
		float DeltaU2 = v2.uv.x - v0.uv.x;
		float DeltaV2 = v2.uv.y - v0.uv.y;

		float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		glm::vec4 tangent;

		tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);


		// Store them
		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent; 

		*/
		//*/
	}

	for (int i = 0; i < indices.size(); i += 3) {
		Vertex& A = p[indices[i]];
		Vertex& B = p[indices[i + 1]];
		Vertex& C = p[indices[i + 2]];

		A.tangent = glm::normalize(A.tangent);
		B.tangent = glm::normalize(B.tangent);
		C.tangent = glm::normalize(C.tangent);
	}
}

std::vector<uint32_t> ge::Assets::load_index_data(Mesh::ptr m, tinygltf::Model const& model, tinygltf::Primitive const& primitive)
{
	if (primitive.indices < 0) {
		std::cerr << "Primitive does not contain indices." << std::endl;
		return {};
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
	return indices;
}

void PrintTextureType(const tinygltf::Model& model) {
	for (size_t i = 0; i < model.materials.size(); ++i) {
		const tinygltf::Material& material = model.materials[i];
		std::cout << "Material " << i << ": " << material.name << std::endl;

		// Check if the material has an albedo texture
		if (material.pbrMetallicRoughness.baseColorTexture.index != -1) {
			int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
			std::cout << "  - Albedo: Texture " << textureIndex << " ("
				<< model.textures[textureIndex].name << ")\n";
		}

		// Check if the material has a metallic-roughness texture
		if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
			int textureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
			std::cout << "  - Metallic-Roughness: Texture " << textureIndex << " ("
				<< model.textures[textureIndex].name << ")\n";
		}

		// Check if the material has a normal texture
		if (material.normalTexture.index != -1) {
			int textureIndex = material.normalTexture.index;
			std::cout << "  - Normal: Texture " << textureIndex << " ("
				<< model.textures[textureIndex].name << ")\n";
		}

		// Check if the material has an occlusion texture
		if (material.occlusionTexture.index != -1) {
			int textureIndex = material.occlusionTexture.index;
			std::cout << "  - Occlusion: Texture " << textureIndex << " ("
				<< model.textures[textureIndex].name << ")\n";
		}

		// Check if the material has an emissive texture
		if (material.emissiveTexture.index != -1) {
			int textureIndex = material.emissiveTexture.index;
			std::cout << "  - Emissive: Texture " << textureIndex << " ("
				<< model.textures[textureIndex].name << ")\n";
		}
	}
}
void ge::Assets::init_materials(ge::CommandBuffer &co)
{
	materials.resize(to_be_loaded.size());

	for (int i = 0; i < to_be_loaded.size(); i++) {
		auto& model = to_be_loaded[i];
		PrintTextureType(model);
		for (const auto& material : model.materials) {
			if (material.normalTexture.index >= 0)
				load_gltf_texture(model, co, materials[i].normal, material.normalTexture.index);
			if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
				load_gltf_texture(model, co, materials[i].albedo, material.pbrMetallicRoughness.baseColorTexture.index);
			if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
				load_gltf_texture(model, co, materials[i].metallic, material.pbrMetallicRoughness.metallicRoughnessTexture.index);
			if (material.occlusionTexture.index >= 0)
				load_gltf_texture(model, co, materials[i].occlusion, material.occlusionTexture.index);
			if (material.emissiveTexture.index >= 0)
				load_gltf_texture(model, co, materials[i].emissive, material.emissiveTexture.index);
		}
	}
}

void ge::Assets::load_gltf_texture(tinygltf::Model const& model, ge::CommandBuffer &co, ge::Image &m, int id)
{
	const tinygltf::Texture& texture = model.textures[id];

	if (texture.source < 0) 
		throw std::runtime_error("Error: image data source error!");

	const tinygltf::Image& image = model.images[texture.source];

	std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
	std::cout << "Component Count: " << image.component << std::endl; // 3 (RGB) or 4 (RGBA)

	if (!image.image.data() || image.image.size() == 0)
		throw std::runtime_error("Error: Color map image data is empty!");

	m.init_raw(
		co,
		image.image.data(), image.width, image.height, (uint32_t)image.image.size(),  // Corrected width calculation
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
}

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

