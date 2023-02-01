#include "Window.hpp"

#include "Logger.hpp"
#include "Graphics.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <thread>
#include <mutex>
#include <vector>

static volatile bool loop_run_flag = false;
static std::mutex input_mutex;

static struct {
	GLFWwindow *glfw_ptr;
	bool resized;
	glm::ivec2 dims;
	struct key_event { int key, scancode, action, mods; };
	std::vector<key_event> key_events;
	struct {
		glm::vec2 old_pos, pos;
		bool initialised;
	} cursor;
} window;

static CameraRot camera;

static void error_callback(int err, const char *desc) {
	logger("GLFW error ", err, ": ", desc, ".");
}
static void framebuffer_size_callback(GLFWwindow *window_ptr, int width, int height) {
	std::lock_guard<std::mutex> guard{ input_mutex };
	if (window_ptr != window.glfw_ptr)
		logger("Unknown window ", window_ptr, " calling framebuffer_size_callback (main window is ", window.glfw_ptr, ").");
	window.dims = { width, height };
	window.resized = true;
}
static void key_callback(GLFWwindow *window_ptr, int key, int scancode, int action, int mods) {
	std::lock_guard<std::mutex> guard{ input_mutex };
	if (window_ptr != window.glfw_ptr)
		logger("Unknown window ", window_ptr, " calling key_callback (main window is ", window.glfw_ptr, ").");
	window.key_events.push_back({ key, scancode, action, mods });
}
static void cursor_position_callback(GLFWwindow *window_ptr, double xpos, double ypos) {
	std::lock_guard<std::mutex> guard{ input_mutex };
	if (window_ptr != window.glfw_ptr)
		logger("Unknown window ", window_ptr, " calling cursor_position_callback (main window is ", window.glfw_ptr, ").");
	window.cursor.pos = { xpos, ypos };
	if (!window.cursor.initialised) {
		window.cursor.old_pos = window.cursor.pos;
		window.cursor.initialised = true;
	}
}

bool Window::init(int width, int height, const char *title) {
	if (window.glfw_ptr) {
		logger("Window has already been initialised.");
		return false;
	}
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		logger("Failed to initialise GLFW.");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window.glfw_ptr = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (!window.glfw_ptr) {
		logger("Failed to open GLFW window.");
		glfwTerminate();
		return false;
	}

	glfwSetInputMode(window.glfw_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(window.glfw_ptr);
	glfwSetFramebufferSizeCallback(window.glfw_ptr, framebuffer_size_callback);
	glfwSetKeyCallback(window.glfw_ptr, key_callback);
	glfwSetCursorPosCallback(window.glfw_ptr, cursor_position_callback);

	logger("Successfully initialised GLFW.");

	if (!Graphics::init()) {
		logger("Failed to initialize graphics.");
		return false;
	}

	window.dims = { width, height };
	window.resized = true;

	return true;
}

void Window::deinit() {
	if (!window.glfw_ptr) {
		logger("Window has not been initialised.");
		return;
	}
	Graphics::deinit();
	glfwDestroyWindow(window.glfw_ptr);
	window.glfw_ptr = nullptr;
	glfwTerminate();
	logger("Successfully deinitialised GLFW.");
}

const uint64_t TARGET_TPS = 60;
const double TARGET_SPT = 1.0 / double(TARGET_TPS);

static void loop_function() {
	logger("Started window loop.");
	glfwMakeContextCurrent(window.glfw_ptr);

	double last_second = glfwGetTime(), last_loop = last_second, tick_time_passed = 0.0;
	uint64_t frame_count = 0, tick_count = 0, fps_display = 0, tps_display = 0;

	while (loop_run_flag) {
		const double current_time = glfwGetTime();
		tick_time_passed += current_time - last_loop;

		if (tick_time_passed >= TARGET_SPT) {
			// Tick
			do {
				tick_count++;
				tick_time_passed -= TARGET_SPT;

				{
					std::lock_guard<std::mutex> guard{ input_mutex };
					if (window.resized) {
						Graphics::resize(window.dims);
						window.resized = false;
					}
					static bool key_w = false, key_s = false, key_a = false, key_d = false,
						key_space = false, key_left_shift = false, key_left_control = false;
					for (auto e : window.key_events) {
						switch (e.key) {
						case GLFW_KEY_ESCAPE:
							if (e.action == GLFW_PRESS)
								glfwSetWindowShouldClose(window.glfw_ptr, GL_TRUE);
							break;
						case GLFW_KEY_W: key_w = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_S: key_s = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_A: key_a = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_D: key_d = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_SPACE: key_space = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_LEFT_SHIFT: key_left_shift = e.action != GLFW_RELEASE; break;
						case GLFW_KEY_LEFT_CONTROL: key_left_control = e.action != GLFW_RELEASE; break;
						}
					}
					window.key_events.clear();

					const glm::vec2 yaw_pitch = 0.01f * (window.cursor.old_pos - window.cursor.pos);
					if (yaw_pitch != glm::vec2{}) camera.rotate(yaw_pitch);
					window.cursor.old_pos = window.cursor.pos;

					const float speed = key_left_control ? 0.5f : 0.1f;
					glm::vec3 move{};
					if (key_w) move.z -= speed;
					if (key_s) move.z += speed;
					if (key_a) move.x -= speed;
					if (key_d) move.x += speed;
					if (key_space) move.y += speed;
					if (key_left_shift) move.y -= speed;
					if (move != glm::vec3{}) camera.move(move);
				}
			} while (tick_time_passed >= TARGET_SPT);

			// Frame
			frame_count++;

			Graphics::render(&camera);

			glfwSwapBuffers(window.glfw_ptr);
		}

		// Trigger each second
		if (current_time - last_second >= 1.0) {
			last_second = current_time;
			fps_display = frame_count;
			tps_display = tick_count;
			frame_count = 0;
			tick_count = 0;
			if (fps_display != TARGET_TPS || tps_display != TARGET_TPS)
				logger("FPS: ", fps_display, ", TPS: ", tps_display);
		}
		last_loop = current_time;
	}

	glfwMakeContextCurrent(nullptr);
	logger("Finishing window loop.");
}

void Window::run() {
	if (!window.glfw_ptr) {
		logger("Window has not been initialised.");
		return;
	}
	glfwMakeContextCurrent(nullptr);

	logger("Starting window loop.");
	loop_run_flag = true;
	std::thread loop_thread{ &loop_function };

	while (!glfwWindowShouldClose(window.glfw_ptr))
		glfwWaitEvents();

	logger("Telling window loop to finish.");
	loop_run_flag = false;
	loop_thread.join();

	logger("Finished window loop.");
	glfwMakeContextCurrent(window.glfw_ptr);
}
