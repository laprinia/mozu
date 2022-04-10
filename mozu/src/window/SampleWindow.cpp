#include "SampleWindow.h"

SampleWindow::SampleWindow(unsigned int width, unsigned int height, const std::string& windowTitle)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 32);
	this->width = width;
	this->height = height;
	window = glfwCreateWindow(width, height, windowTitle.c_str(), NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window" << std::endl;
		glfwTerminate();
	}
	GLFWimage icon;
	icon.pixels = SOIL_load_image(".\\..\\mozu\\src\\resources\\icon.png", &icon.width, &icon.height, 0, SOIL_LOAD_RGBA);
	glfwSetWindowIcon(window, 1, &icon);
	SOIL_free_image_data(icon.pixels);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
	}
	glViewport(0, 0, width, height);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	SampleWindow::AddShaders();
	SampleWindow::Init();
	while (!glfwWindowShouldClose(window))
	{
		SampleWindow::Update();
	}

	glfwTerminate();
}


void SampleWindow::Init()
{
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	float data[] = {
		-1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STATIC_DRAW);
	const GLint posPtr = glGetAttribLocation(shaders["base"], "position");
	std::cout << "Position for pos in shader: " << posPtr << std::endl;
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);
}

void SampleWindow::Update()
{
	glfwPollEvents();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["base"]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glfwSwapBuffers(window);

}

void SampleWindow::AddShaders()
{
	shaders["base"] = ShaderManager::AddShader("Base");
}

int SampleWindow::GetWindowHeight()
{
	return height;
}

int SampleWindow::GetWindowWidth()
{
	return width;
}

SampleWindow::~SampleWindow()
{
	//clear data
}