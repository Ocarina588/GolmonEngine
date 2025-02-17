#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GolmonEngine.hpp"

// MIKKSTPACE

#include <assimp/Importer.hpp>      // Assimp importer class
#include <assimp/scene.h>  
#include <assimp/postprocess.h>// Main data structure for scenes
#include "Assets/mikktspace.h"

struct mikktspace_data {
	ge::Vertex* vertices = nullptr;
	uint32_t* indices = nullptr;
	size_t size = 0;
};

std::vector<ge::Mesh::ptr> ge::Assets::meshes;
std::vector<ge::Assets::material_s> ge::Assets::materials;
std::vector<ge::Image> ge::Assets::textures;
std::vector<ge::Assets::texture_s> ge::Assets::to_be_loaded; 

// MikkTSpace callbacks
static int getNumFaces(const SMikkTSpaceContext* pContext) {
	mikktspace_data* mesh = (mikktspace_data*)pContext->m_pUserData;
	return (int)mesh->size / 3; // Assuming a triangle mesh
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
	textures.clear();
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

uint8_t* ge::Assets::load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
}

uint8_t* ge::Assets::load_from_file(char const* file, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load(file, x, y, comp, req_comp);
}

void ge::Assets::load_assimp(char const* file_name)
{
		Assimp::Importer importer;
		aiScene const* scene = importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!scene)
			throw std::runtime_error(importer.GetErrorString());
	
		load_assimp_vertices(scene);
		load_assimp_materials(scene);

		//meshes[i].init(meshes[i].vertices.data(), sizeof(Vertex) * meshes[i].vertices.size());


		//if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, *(aiColor3D*)&materials[mesh->mMaterialIndex].diffuse) != AI_SUCCESS)
		//	materials[mesh->mMaterialIndex].diffuse = { 1.f, 0.f, 0.f };
		//if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, *(aiColor3D*)&materials[mesh->mMaterialIndex].specular) != AI_SUCCESS)
		//	materials[mesh->mMaterialIndex].specular = { 0.f, 0.f, 0.f };
		//if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_EMISSIVE, *(aiColor3D*)&materials[mesh->mMaterialIndex].emissive) != AI_SUCCESS)
		//	materials[mesh->mMaterialIndex].specular = { 0.f, 0.f, 0.f };

		//meshes[i].material_index = mesh->mMaterialIndex;
		//std::cout << i << " " << mesh->mMaterialIndex << std::endl;
		//materials[mesh->mMaterialIndex].name = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();

}

void ge::Assets::load_model(char const* file)
{
	clear();
	load_assimp(file);
}

void ge::Assets::load_assimp_vertices(void const* s)
{
	aiScene const* scene = (aiScene const *)s;
	// im doing a vector of vectors because I want to normalize the size model, 
	// so I need to get the length of all the meshes of the model before loading them into the gpu
	std::vector<std::vector<ge::Vertex>> vertices(scene->mNumMeshes); 
	std::vector<std::vector<uint32_t>> indices(scene->mNumMeshes);
	glm::vec3 min_pos(*(glm::vec3*)&scene->mMeshes[0]->mVertices[0]), max_pos(*(glm::vec3*)&scene->mMeshes[0]->mVertices[0]);
	float max_length = 0;

	if (scene->mNumMeshes <= 0)
		throw std::runtime_error("model doest have any meshes");

	for (int i = 0; i < (int)scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		
		for (int j = 0; j < (int)mesh->mNumFaces; j++)
			for (int k = 0; k < 3; k++)
				indices[i].push_back(mesh->mFaces[j].mIndices[k]);

		vertices[i].resize(mesh->mNumVertices);
		for (int j = 0; j < (int)mesh->mNumVertices; j++) {
			vertices[i][j].pos = *(glm::vec3*)(&mesh->mVertices[j]);
			vertices[i][j].normal = *(glm::vec3*)(&mesh->mNormals[j]);
			vertices[i][j].tangent = *(glm::vec3*)(&mesh->mTangents[j]);
			vertices[i][j].bitangent = *(glm::vec3*)(&mesh->mBitangents[j]);
			vertices[i][j].uv = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };

			max_length = std::max(glm::length(vertices[i][j].pos), max_length);
			min_pos = glm::min(min_pos, vertices[i][j].pos);
			max_pos = glm::max(max_pos, vertices[i][j].pos);
		}
	}

	glm::vec3 center = (min_pos + max_pos) / 2.0f;

	for (int i = 0; i < (int)scene->mNumMeshes; i++) {

		for (auto& v : vertices[i])
			v.pos = (v.pos - center) / max_length;
	
		ge::Mesh::ptr mesh = std::make_shared<ge::Mesh>(); 
		mesh->material_id = scene->mMeshes[i]->mMaterialIndex;
		mesh->load_indices(indices[i]);
		mesh->load_vertices(vertices[i]);
		meshes.push_back(mesh); 
	}
}

void ge::Assets::load_assimp_materials(void const* s)
{
	aiScene const* scene = (aiScene const*)s;

	auto get_material_texture_index = [&](aiMaterial const* material, aiTextureType type) -> uint32_t {
		if (material->GetTextureCount(type) <= 0) return 0;
		aiString texture_path;
		if (material->GetTexture(type, 0, &texture_path) != aiReturn_SUCCESS)
			throw std::runtime_error("internal error while loading textures");
		std::string name = texture_path.C_Str();
		if (name[0] != '*')
			throw std::runtime_error("no support for not emedded textures yet");
		return std::stoi(name.substr(1)) + 1;
	};

	for (int i = 0; i < (int)scene->mNumTextures; i++) {
		texture_s texture;
		if (scene->mTextures[i]->mHeight != 0)
			throw std::runtime_error("we dont support non embedded textures yet");
		texture.data = ge::Assets::load_from_memory(
			(stbi_uc*)scene->mTextures[i]->pcData, scene->mTextures[i]->mWidth, 
			&texture.w, &texture.h, &texture.c, STBI_rgb_alpha
		);
		to_be_loaded.push_back(texture);
	}

	for (int i = 0; i < (int)scene->mNumMaterials; i++) {
		auto const material = scene->mMaterials[i];
		material_s m; 
		m.index_albedo = get_material_texture_index(material, aiTextureType_BASE_COLOR);
		m.index_metallic = get_material_texture_index(material, aiTextureType_METALNESS);
		m.index_normal = get_material_texture_index(material, aiTextureType_NORMALS);
		m.index_occlusion = get_material_texture_index(material, aiTextureType_LIGHTMAP);
		m.index_emissive = get_material_texture_index(material, aiTextureType_EMISSIVE);
		materials.push_back(m); 
	}
}


void ge::Mesh::load_indices(std::vector<unsigned int> const& indices)
{
	index_data.init(
		(void*)indices.data(), (uint32_t)(sizeof(uint32_t) * indices.size()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void ge::Mesh::load_vertices(std::vector<ge::Vertex> const& vertices)
{
	vertex_data.init(
		(void *)vertices.data(), (uint32_t)(sizeof(ge::Vertex) * vertices.size()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void ge::Assets::upload_textures(ge::CommandBuffer& co)
{
	for (auto const &i : to_be_loaded) {
		textures.emplace_back();
		textures.rbegin()->init_raw(co,
			i.data, i.w, i.h, i.w * i.h * 4 * sizeof(uint8_t),
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_IMAGE_ASPECT_COLOR_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	}
}