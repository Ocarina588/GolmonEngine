#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Objects/Buffer.hpp"

namespace ge {
	struct UBO {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 light_pos; // temporal, to be removed
		glm::vec4 view_pos;
		uint32_t index_albedo;
		uint32_t index_normal;
		uint32_t index_metallic;
		uint32_t index_emissive;
		uint32_t index_occlusion;
	};

	class Camera {
	public:
		Camera(void);
		~Camera(void);

		operator VkBuffer () { return buffer; };

		void init(void);
		void update(void);
		void process_mouse(double x, double y);
		void process_scroll(double x, double y);
		void write_ubo(void);

		static glm::vec3 rotate_around_point(const glm::vec3& point, const glm::vec3& center, glm::vec3 axis, float angle);

		UBO ubo;
		ge::Buffer buffer;
	private:

		glm::vec3 pos;
		glm::vec3 target;
		glm::vec3 direction;
		glm::vec3 world_up = { 0.f, 1.f, 0.f };
		glm::vec3 right;
	};
}