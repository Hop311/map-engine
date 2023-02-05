
#include "Window.hpp"
#include "Logger.hpp"

#include <glm/glm.hpp>

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning( disable : 4201 )
	#include <glm/gtx/string_cast.hpp>
	#pragma warning( pop ) 
#else
	#include <glm/gtx/string_cast.hpp>
#endif

int main(void) {
	if (!Window::init(1920, 1080, "sphere-map")) {
		logger("Window initialisation failed.");
		return -1;
	}
	Window::run();
	Window::deinit();
	return 0;
}
