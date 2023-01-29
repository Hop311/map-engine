# 3.15 fixes "overriding '/W3' with '/W4'" warning for MSVC
cmake_minimum_required(VERSION 3.15)

project(map-engine)

set(SOURCES "source/Main.cpp" "source/Logger.cpp" "source/Window.cpp"
	"source/Graphics.cpp" "source/GLTools.cpp" "source/Camera.cpp")

# Executable and compile options
add_executable(map-engine ${SOURCES})
set_target_properties(map-engine PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED True)
if(MSVC)
	target_compile_options(map-engine PRIVATE "/W4;/WX;$<$<CONFIG:RELEASE>:/O2>")
else()
	target_compile_options(map-engine PRIVATE "-Wall;-Wextra;-Werror;$<$<CONFIG:RELEASE>:-O3>")
endif()

# Dependencies
add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)
add_definitions(-DGLEW_STATIC)
option(glew-cmake_BUILD_SHARED NO)
add_subdirectory(deps/glew EXCLUDE_FROM_ALL)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/soil EXCLUDE_FROM_ALL)
target_link_libraries(map-engine PRIVATE glfw PRIVATE libglew_static PRIVATE glm PRIVATE soil)
