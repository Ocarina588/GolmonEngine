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

	class Mesh {
	public:
		Mesh(void);
		~Mesh(void);

		void load_from_glb(char const* file);
		void draw(ge::CommandBuffer& command_buffer);
	private:
		std::vector<ge::Buffer> vertex_data;
		std::vector<ge::Buffer> index_data;

		void load_vertex_data(tinygltf::Model const &model, tinygltf::Primitive const & primitive);
		void load_index_data(tinygltf::Model const& model, tinygltf::Primitive const& primitive);
	};


}