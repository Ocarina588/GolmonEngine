#include "GolmonEngine.hpp"

ge::Camera::Camera(void)
{

}

ge::Camera::~Camera(void)
{

}

void ge::Camera::init(void)
{
	pos = { 0.f, 0.f, 3.f };
	target = { 0.f, 0.f, 0.f };

	ubo.proj = glm::perspective(glm::radians(45.f), ge::ctx::device.extent.width /
		(float)ge::ctx::device.extent.height, 0.01f, 1000.0f);
	ubo.proj[1][1] *= -1;
	ubo.model = glm::mat4(1.f);
	buffer.init(
		&ubo, sizeof(ge::UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	buffer.map();
}

void ge::Camera::update(void)
{
	static glm::vec3 light_pos = { 0.f, 0.f, 5.f };
	static float dt = 0;
	dt += (float)ge::ctx::window.dt;
	direction = glm::normalize(target - pos);
	right = glm::normalize(glm::cross(world_up, direction));
	light_pos = { 0.f, 0.f, 5.f };
	light_pos = rotate_around_point(light_pos, target, { 0.f, 1.f, 0.f }, -(float)0.5 * dt * glm::radians(90.f));
	light_pos = pos;
	ubo.light_pos = glm::vec4(light_pos, 1.f);
	ubo.view_pos = glm::vec4(pos, 1.f);
	ubo.view = glm::lookAt(pos, target, world_up);
	ubo.model = glm::mat4(1.f);
	//ubo.model = glm::rotate(ubo.model, 0.5f * dt * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.model = glm::rotate(ubo.model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
	//ge::Camera::rotate_around_point(ubo.light_pos)
	write_ubo();
}

void ge::Camera::write_ubo(void)
{
	buffer.memcpy(&ubo, sizeof(ge::UBO));
}

glm::vec3 ge::Camera::rotate_around_point(const glm::vec3& point, const glm::vec3& center, glm::vec3 axis, float angle) {
	glm::vec3 translatedPoint = point - center;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
	glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(translatedPoint, 1.0f);

	return glm::vec3(rotatedPoint) + center;
}

void ge::Camera::process_mouse(double x, double y)
{
	float speed = 3.f;
	float dt = (float)ge::ctx::window.dt;
	
	pos = rotate_around_point(pos, target, { 0.f, 1.f, 0.f }, -(float)x * speed * dt * glm::radians(90.f));
	pos = rotate_around_point(pos, target, right, (float)y * speed * dt * glm::radians(90.f));
	//ubo.model = glm::rotate(ubo.model, (float)x * speed * dt * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	//ubo.model = glm::rotate(ubo.model, (float)y * speed * dt * glm::radians(90.f), right);

}

void ge::Camera::process_scroll(double x, double y)
{
	float speed = 30.f;
	if (y > 0)
		pos += direction * speed * (float)ge::ctx::window.dt;
	if (y < 0)
		pos += direction * speed * (float)-ge::ctx::window.dt;
}