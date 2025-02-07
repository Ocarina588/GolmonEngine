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
		glm::vec3 light_direction; // temporal, to be removed
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

		static glm::vec3 rotate_around_point(const glm::vec3& point, const glm::vec3& center, glm::vec3 axis, float angle);

	private:
		UBO ubo;
		ge::Buffer buffer;

		glm::vec3 pos;
		glm::vec3 target;
		glm::vec3 direction;
		glm::vec3 world_up = { 0.f, 1.f, 0.f };
		glm::vec3 right;
	};
}