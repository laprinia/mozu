#pragma once
#define GLEW_STATIC
#include <iostream>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_aug.h>
#include <SOIL.h>
#include "gtc/matrix_transform.hpp"

#include "../shaders/ShaderManager.h"
#include "../input/InputManager.h"
#include "../camera/Camera.h"
#include "../renderables/SimpleMesh.h"


class SampleWindow
{
private:
	GLFWwindow* window;
	unsigned int width, height;
	std::unordered_map<std::string, GLuint> shaders;
	static Camera* camera;
	SimpleMesh* quadMesh;
	static bool firstMouseMove;
	static double lastMouseX, lastMouseY;
	static double yaw, pitch;
	static float mouseSensitivity;
	static float cameraSpeed;
	static float deltaTime;
	static float lastFrame;
	unsigned int* fbID;
	unsigned int* bufferTexture;
	bool hasCameraMoved = false;
	void Init();
	void Update();
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