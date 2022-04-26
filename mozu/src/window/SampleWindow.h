#pragma once
#define GLEW_STATIC
#include <iostream>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_aug.h>
#include <SOIL.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "gtc/matrix_transform.hpp"
#include <gtc/type_ptr.hpp>


#include "../shaders/ShaderManager.h"
#include "../input/InputManager.h"
#include "../gui/GUIManager.h"
#include "../camera/Camera.h"


class SampleWindow
{
private:
	GLFWwindow* window;
	static unsigned int width, height;
	std::unordered_map<std::string, GLuint> shaders;
	static Camera* camera;
	unsigned int computeTexture;
	unsigned int quadVAO;
	static bool hasCameraMoved;
	static bool hasTriggeredMovement;
	static bool firstMouseMove;
	static double lastMouseX, lastMouseY;
	static double yaw, pitch;
	static float mouseSensitivity;
	static float cameraSpeed;
	static float deltaTime;
	static float lastFrame;
	unsigned int* fbID;
	unsigned int* bufferTexture;
	int frameNumber = 0;

	static bool hasGUI;
	static int samples;
	static int maxDepth;
	void Init();
	void Update();
	void GUIUpdate();
	void PathTrace();
	void RenderPathResult();
	void AddShaders();

public:
	SampleWindow(unsigned int width, unsigned int height, const std::string& windowTitle);

	static void OnKeyPress(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void OnCursorPositionChange(GLFWwindow* window, double xPosition, double yPosition);
	static void OnScrollChange(GLFWwindow* window, double xOffset, double yOffset);
	void OnInputUpdate();
	static void OnFramebufferSizeChange(GLFWwindow* window, int width, int height);

	int GetWindowHeight();
	int GetWindowWidth();

	~SampleWindow();

};