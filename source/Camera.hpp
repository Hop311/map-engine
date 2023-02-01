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
	glm::mat4 matrix;
	glm::vec3 pos;
protected:
	glm::vec3 front, right;
public:
	CameraFree();
	CameraFree(glm::vec3 position, glm::vec3 facing);

	void move(glm::vec3 delta) override;
	void rotate(glm::vec2 yaw_pitch_rads) override;
	void updateMatrix() override;
	glm::mat4 getMatrix() const override;
};

class CameraRot : public CameraFree {
	glm::vec2 yaw_pitch;
public:
	CameraRot();
	CameraRot(glm::vec3 position, glm::vec2 yaw_pitch_rads);

	void rotate(glm::vec2 yaw_pitch_rads) override;
	void updateMatrix() override;
};
