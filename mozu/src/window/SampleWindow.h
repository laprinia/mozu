#pragma once
#define GLEW_STATIC
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_aug.h>
#include <SOIL.h>

class SampleWindow
{
private:
	GLFWwindow* window;
	unsigned int width, height;

	void Init();
	void Update();

public:
	SampleWindow(unsigned int width, unsigned int height, const std::string& windowTitle);
	~SampleWindow();

	int GetWindowHeight();
	int GetWindowWidth();


};

