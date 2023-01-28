#pragma once

#include "Camera.hpp"

namespace Graphics {
	bool init();
	void deinit();
	void render(const Camera *camera);
	void resize(glm::ivec2 dims);
}
