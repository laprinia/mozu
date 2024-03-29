#include "SampleWindow.h"

unsigned int SampleWindow::width = 0;
unsigned int SampleWindow::height = 0;
bool SampleWindow::hasGUI = false;
Camera* SampleWindow::camera = nullptr;
bool SampleWindow::hasCameraMoved = false;
bool SampleWindow::hasTriggeredMovement = false;
float SampleWindow::cameraSpeed = 20.0f;
float SampleWindow::deltaTime = 0.0f;
float SampleWindow::lastFrame = 0.0f;
bool SampleWindow::firstMouseMove = true;
double SampleWindow::lastMouseX = 0.0f;
double SampleWindow::lastMouseY = 0.0f;
double SampleWindow::yaw = -90.0f;
double SampleWindow::pitch = 0.0f;
float SampleWindow::mouseSensitivity = 0.1f;
float SampleWindow::lightExposure = 0.5f;

int SampleWindow::samples = 1;
int SampleWindow::maxDepth = 50;
bool SampleWindow::hasBloom = false;
float SampleWindow::threshold = 0.7;
float SampleWindow::strength = 0.5;
float SampleWindow::radius = 5.0;


SampleWindow::SampleWindow(unsigned int width, unsigned int height, const std::string& windowTitle)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 32);
	this->width = width; this->height = height;
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

	SampleWindow::Init();

	while (!glfwWindowShouldClose(window))
	{
		SampleWindow::Update();
	}
	GUIManager::DeleteContext();
	glfwTerminate();
}


void SampleWindow::Init()
{
	GUIManager::CreateContext(window);
	camera = new Camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	float vertices[] = {
		1.f,  1.f, 0.0f,  1.f,  1.f,
		1.f, -1.f, 0.0f,  1.f,  0.f,
	   -1.f, -1.f, 0.0f,  0.f,  0.f,
	   -1.f,  1.f, 0.0f,  0.f,  1.f,
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glGenFramebuffers(1, &computeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, computeFBO);
	glGenTextures(1, &computeTexture);
	glBindTexture(GL_TEXTURE_2D, computeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glGenFramebuffers(1, &threshFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, threshFBO);
	glGenTextures(1, &thresholdTexture);
	glBindTexture(GL_TEXTURE_2D, thresholdTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thresholdTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glGenTextures(1, &blurTexture);
	glBindTexture(GL_TEXTURE_2D, blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &bloomFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	glGenTextures(1, &bloomTexture);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	SampleWindow::AddShaders();
}

void SampleWindow::Update()
{
	!hasGUI ? glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) : glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwPollEvents();
	OnInputUpdate();
	GUIUpdate();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	SampleWindow::PathTrace();
	SampleWindow::RenderThreshResult();
	SampleWindow::RenderBlurResult();
	SampleWindow::RenderFinalResult();
	hasCameraMoved = false;
	hasTriggeredMovement = false;
	GUIManager::DrawData();
	glfwSwapBuffers(window);

}

void SampleWindow::GUIUpdate() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (hasGUI)
	{
		GUIManager::DrawSampleData(&samples, &maxDepth, &hasBloom,&threshold,&strength,&radius);
	}

	ImGui::EndFrame();

}

void SampleWindow::PathTrace() {
	glBindFramebuffer(GL_FRAMEBUFFER, computeFBO);
	if (hasCameraMoved) frameNumber = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["trace"]);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(camera->getFieldOfView()), (float)width / (float)height, 0.1f, 800.0f);
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(camera->getCameraPosition(), camera->getCameraPosition() + camera->getCameraFront(),
		camera->getCameraUp());

	glUniformMatrix4fv(glGetUniformLocation(shaders["trace"], "invProjection"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
	glUniformMatrix4fv(glGetUniformLocation(shaders["trace"], "invView"), 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));

	glUniform1f(glGetUniformLocation(shaders["trace"], "aspectRatio"), float(width) / float(height));
	glm::vec2 resolution = glm::vec2(float(width), float(height));
	glUniform1f(glGetUniformLocation(shaders["trace"], "fov"), camera->getFieldOfView());
	glUniform2fv(glGetUniformLocation(shaders["trace"], "resolution"), 1, glm::value_ptr(resolution));
	glUniform1f(glGetUniformLocation(shaders["trace"], "frameNo"), frameNumber);
	glUniform1f(glGetUniformLocation(shaders["trace"], "accum"), float(frameNumber) / float(frameNumber + 1));

	glUniform1i(glGetUniformLocation(shaders["trace"], "samples"), samples);
	glUniform1i(glGetUniformLocation(shaders["trace"], "maxDepth"), maxDepth);

	
	
	glBindImageTexture(0, computeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(width, height, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	frameNumber++;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SampleWindow::RenderThreshResult() {

	glBindFramebuffer(GL_FRAMEBUFFER, threshFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["thresh"]);
	glUniform1f(glGetUniformLocation(shaders["thresh"], "threshold"), threshold);
	glUniform1f(glGetUniformLocation(shaders["thresh"], "strength"), strength);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, computeTexture);
	glUniform1i(glGetUniformLocation(shaders["thresh"], "RTtexture"), 0);
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SampleWindow::RenderBlurResult() {

	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["blur"]);
	glUniform1f(glGetUniformLocation(shaders["blur"], "resolution"), width);
	glUniform1f(glGetUniformLocation(shaders["blur"], "radius"), radius);
	glUniform2f(glGetUniformLocation(shaders["blur"], "direction"), 1.0, 0.0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, thresholdTexture);
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(glGetUniformLocation(shaders["blur"], "resolution"), height);
	glUniform1f(glGetUniformLocation(shaders["blur"], "radius"), radius);
	glUniform2f(glGetUniformLocation(shaders["blur"], "direction"), 0.0, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blurTexture);
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void SampleWindow::RenderFinalResult() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders["post"]);
	glUniform1f(glGetUniformLocation(shaders["post"], "lightExposure"), lightExposure);
	glUniform1ui(glGetUniformLocation(shaders["post"], "bloom"), hasBloom);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, computeTexture);

	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);

	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	GUIManager::DrawData();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SampleWindow::AddShaders()
{
	shaders["base"] = ShaderManager::AddBaseShader("Base");
	shaders["post"] = ShaderManager::AddBaseShader("Post");
	shaders["trace"] = ShaderManager::AddComputeShader("Trace");
	shaders["thresh"] = ShaderManager::AddBaseShader("Thresh");
	shaders["blur"] = ShaderManager::AddBaseShader("Blur");
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
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		hasGUI ^= true;
		if (hasGUI == false) {

			glfwSetCursorPos(window, width / 2, height / 2);
		}
	}
}

void SampleWindow::OnCursorPositionChange(GLFWwindow* window, double xPosition, double yPosition) {
	if (!hasGUI) {
		hasTriggeredMovement = true;
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
}

void SampleWindow::OnScrollChange(GLFWwindow* window, double xOffset, double yOffset) {

	hasTriggeredMovement = true;
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
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() + (actualSpeed * camera->getCameraUp()));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		isCameraMovement = true;
		camera->setCameraPosition(camera->getCameraPosition() - (actualSpeed * camera->getCameraUp()));
	}

	isCameraMovement || hasTriggeredMovement ? hasCameraMoved = true : hasCameraMoved = false;

}

void SampleWindow::OnFramebufferSizeChange(GLFWwindow* window, int width, int height) {

	glViewport(0, 0, width, height);
}

SampleWindow::~SampleWindow()
{
	//clear data
}