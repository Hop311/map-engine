#include "Camera.hpp"

#include "Logger.hpp"

#include <glm/gtx/rotate_vector.hpp>

#include <numbers>

const glm::vec3 FORWARDS{ 0.0f, 0.0f, -1.0f }, UP{ 0.0f, 1.0f, 0.0f };

CameraFree::CameraFree(void) : CameraFree{ glm::vec3{}, { 0.0f, 0.0f, -1.0f } } {}
CameraFree::CameraFree(glm::vec3 position, glm::vec3 facing) : pos{ position },
	front{ facing == glm::vec3{} ? FORWARDS : glm::normalize(facing) },
	right(glm::cross(front, UP)) {
	if (facing == glm::vec3{})
		logger("Facing vector is 0 (setting to (0,0,-1)).");
	updateMatrix();
}

void CameraFree::move(glm::vec3 delta) {
	pos += -delta.z * front + delta.y * UP + delta.x * right;
	updateMatrix();
}
void CameraFree::rotate(glm::vec2 yaw_pitch_rads) {
	front = glm::rotate(front, yaw_pitch_rads.x, UP);
	right = glm::cross(front, UP);
	front = glm::rotate(front, yaw_pitch_rads.y, right);
	CameraFree::updateMatrix();
}
void CameraFree::updateMatrix(void) {
	matrix = glm::lookAt(pos, front + pos, UP);
}
glm::mat4 CameraFree::getMatrix(void) const {
	return matrix;
}

const float PITCH_LIMIT = std::numbers::pi_v<float> * 0.5f - glm::radians(25.0f);

CameraRot::CameraRot(void) : CameraRot{ glm::vec3{}, glm::vec2{} } {}
CameraRot::CameraRot(glm::vec3 position, glm::vec2 yaw_pitch_rads) :
	CameraFree{ position, FORWARDS }, yaw_pitch{ yaw_pitch_rads } {
	updateMatrix();
}
void CameraRot::rotate(glm::vec2 yaw_pitch_rads) {
	yaw_pitch += yaw_pitch_rads;
	updateMatrix();
}
void CameraRot::updateMatrix(void) {
	if (yaw_pitch.y < -PITCH_LIMIT) yaw_pitch.y = -PITCH_LIMIT;
	else if (yaw_pitch.y > PITCH_LIMIT) yaw_pitch.y = PITCH_LIMIT;
	front = FORWARDS;
	CameraFree::rotate(yaw_pitch);
}
