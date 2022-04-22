#pragma once
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <vector>
#include <string>


class GUIManager {
private:
	static GLFWwindow* windowContext;
	GUIManager() = default;
	GUIManager(GLFWwindow* windowContext);

public:

	static GUIManager& CreateContext(GLFWwindow* windowContext);
	static void DrawData();
	static void DrawSampleData(int* samples);
	static void DeleteContext();
};
