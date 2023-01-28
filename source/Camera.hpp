#pragma once

#include <glm/glm.hpp>

struct Camera {
	virtual ~Camera() = default;

	virtual void move(glm::vec3 delta) = 0;
	virtual void rotate(glm::vec2 yaw_pitch) = 0;
	virtual void updateMatrix() = 0;
	virtual glm::mat4 getMatrix() const = 0;
};

class CameraFree : public Camera {
	glm::vec3 pos, front, up, right;
	glm::mat4 matrix;
public:
	CameraFree();
	CameraFree(glm::vec3 position, glm::vec3 facing);

	void move(glm::vec3 delta) override;
	void rotate(glm::vec2 yaw_pitch) override;
	void updateMatrix() override;
	glm::mat4 getMatrix() const;
};
