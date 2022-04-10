#pragma once
#define GLEW_STATIC
#include <iostream>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_aug.h>
#include <SOIL.h>

#include "../shaders/ShaderManager.h"

class SampleWindow
{
private:
	GLFWwindow* window;
	unsigned int width, height;
	std::unordered_map<std::string, GLuint> shaders;

	void Init();
	void Update();
	void AddShaders();

public:
	SampleWindow(unsigned int width, unsigned int height, const std::string& windowTitle);
	~SampleWindow();

	int GetWindowHeight();
	int GetWindowWidth();


};