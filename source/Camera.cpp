#include "Camera.hpp"

#include "Logger.hpp"

#include <glm/gtx/rotate_vector.hpp>

CameraFree::CameraFree() : CameraFree(glm::vec3(0.0f), {0.0f, 0.0f, -1.0f}) {}
CameraFree::CameraFree(glm::vec3 position, glm::vec3 facing) : pos(position),
	front(facing == glm::vec3{} ? glm::vec3{ 0.0f, 0.0f, -1.0f } : glm::normalize(facing)),
	up(0.0f, 1.0f, 0.0f), right(glm::cross(front, up)) {
	if (facing == glm::vec3{})
		logger("Facing vector is 0 (setting to (0,0,-1)).");
	updateMatrix();
}

void CameraFree::move(glm::vec3 delta) {
	pos += -delta.z * front + delta.y * up + delta.x * right;
	updateMatrix();
}
void CameraFree::rotate(glm::vec2 yaw_pitch) {
	front = glm::rotate(front, yaw_pitch.x, up);
	right = glm::cross(front, up);
	front = glm::rotate(front, yaw_pitch.y, right);
	//up = glm::cross(right, front);
	updateMatrix();
}
void CameraFree::updateMatrix() {
	matrix = glm::lookAt(pos, front + pos, up);
}
glm::mat4 CameraFree::getMatrix() const {
	return matrix;
}
