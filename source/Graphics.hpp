#pragma once

#include "Camera.hpp"

namespace Graphics {
	bool init(void);
	void deinit(void);
	void render(const Camera *camera);
	void resize(glm::ivec2 dims);
	void togggle_draw_3D(void);
}
