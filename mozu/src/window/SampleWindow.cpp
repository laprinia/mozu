#include "SampleWindow.h"

Camera* SampleWindow::camera = nullptr;
float SampleWindow::cameraSpeed = 20.0f;
float SampleWindow::deltaTime = 0.0f;
float SampleWindow::lastFrame = 0.0f;
bool SampleWindow::firstMouseMove = true;
double SampleWindow::lastMouseX = 0.0f;
double SampleWindow::lastMouseY = 0.0f;
double SampleWindow::yaw = -90.0f;
double SampleWindow::pitch = 0.0f;
float SampleWindow::mouseSensitivity = 0.1f;

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
	InputManager::GetInstance(window);
	InputManager::SetFramebufferSizeCallback(window, OnFramebufferSizeChange);
	InputManager::SetWindowKeyCallback(window, OnKeyPress);
	InputManager::SetWindowCursorPositionCallback(window, OnCursorPositionChange);
	InputManager::SetWindowScrollCallback(window, OnScrollChange);
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
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);

	camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	std::vector<SVertex> screenQuadVertices = {

			SVertex{glm::vec3(-1.0f,-1.0f,0.0f),glm::vec2(0.0f,0.0f)},
			SVertex{glm::vec3(-1.0f,1.0f,0.0f),glm::vec2(0.0f,1.0f)},
			SVertex{glm::vec3(1.0f,1.0f,0.0f),glm::vec2(1.0f,1.0f)},
			SVertex{glm::vec3(1.0f,-1.0f,0.0f),glm::vec2(1.0f,0.0f)}
	};

	std::vector<GLuint> screenQuadIndices = { 0, 1, 2,
		0, 2, 3 };
	quadMesh = new SimpleMesh(screenQuadVertices, screenQuadIndices);

	unsigned int fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	fbID = &fb;

	unsigned int bt;
	glGenTextures(1, &bt);
	glBindTexture(GL_TEXTURE_2D, bt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bt, 0);
	bufferTexture = &bt;
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << " Framebuffer isn't complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SampleWindow::Update()
{
	glfwPollEvents();
	OnInputUpdate();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//MVP
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(camera->getCameraPosition(), camera->getCameraPosition() + camera->getCameraFront(),
		camera->getCameraUp());
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(camera->getFieldOfView()), (float)width / (float)height, 0.1f, 800.0f);

	glm::mat4 model = glm::mat4(1.0f);

	SampleWindow::PathTrace();
	SampleWindow::RenderPathResult();
	glfwSwapBuffers(window);

}

void SampleWindow::PathTrace() {
	if (hasCameraMoved) frameNumber = 0;

	frameNumber++;
	glBindFramebuffer(GL_FRAMEBUFFER, *fbID);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["base"]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SampleWindow::RenderPathResult() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *bufferTexture);
	glUseProgram(shaders["post"]);
	quadMesh->Draw();
}

void SampleWindow::AddShaders()
{
	shaders["base"] = ShaderManager::AddShader("Base");
	shaders["post"] = ShaderManager::AddShader("Post");
}

int SampleWindow::GetWindowHeight()
{
	return height;
}

int SampleWindow::GetWindowWidth()
{
	return width;
}

void SampleWindow::OnKeyPress(GLFWwindow* window, int key, int scancode, int action, int mode) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void SampleWindow::OnCursorPositionChange(GLFWwindow* window, double xPosition, double yPosition) {

	if (firstMouseMove) {
		firstMouseMove = false;
		lastMouseX = xPosition;
		lastMouseY = yPosition;
	}
	float xOffset = xPosition - lastMouseX;
	float yOffset = lastMouseY - yPosition;
	lastMouseX = xPosition;
	lastMouseY = yPosition;

	yaw += mouseSensitivity * xOffset;
	pitch += mouseSensitivity * yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = glm::cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = glm::sin(glm::radians(pitch));
	front.z = glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));

	camera->setCameraFront(glm::normalize(front));
}

void SampleWindow::OnScrollChange(GLFWwindow* window, double xOffset, double yOffset) {

	camera->setFieldOfView(camera->getFieldOfView() - yOffset);
	float foV = camera->getFieldOfView();
	if (foV < 1.0f) camera->setFieldOfView(1.0f);
	if (foV > 45.0f) camera->setFieldOfView(45.0f);
}

void SampleWindow::OnInputUpdate() {

	float currentTime = glfwGetTime();
	deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;
	float actualSpeed = cameraSpeed * deltaTime;
	bool isCameraMovement = false;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() + (actualSpeed * camera->getCameraFront()));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() - (actualSpeed * camera->getCameraFront()));
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() - (actualSpeed * (glm::normalize(
			glm::cross(camera->getCameraFront(), camera->getCameraUp())))));
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() + (actualSpeed * (glm::normalize(
			glm::cross(camera->getCameraFront(), camera->getCameraUp())))));
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera->setCameraPosition(camera->getCameraPosition() + (actualSpeed * camera->getCameraUp()));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera->setCameraPosition(camera->getCameraPosition() - (actualSpeed * camera->getCameraUp()));
	}

	isCameraMovement ? hasCameraMoved = true : hasCameraMoved = false;

}

void SampleWindow::OnFramebufferSizeChange(GLFWwindow* window, int width, int height) {

	glViewport(0, 0, width, height);
}

SampleWindow::~SampleWindow()
{
	//clear data
}