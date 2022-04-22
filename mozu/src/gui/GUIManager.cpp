#include "GUIManager.h"
#include <iostream>


GUIManager::GUIManager(GLFWwindow* windowContext) {
	GUIManager::windowContext = windowContext;
};

GLFWwindow* GUIManager::windowContext = nullptr;

GUIManager& GUIManager::CreateContext(GLFWwindow* windowContext) {
	static GUIManager instance(windowContext);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplGlfw_InitForOpenGL(windowContext, true);
	ImGui_ImplOpenGL3_Init("#version 460");
	ImGui::StyleColorsDark();
	return instance;
}

void GUIManager::DeleteContext()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

}

void GUIManager::DrawData()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void GUIManager::DrawSampleData(int* samples)
{
	ImGui::Begin("mozu");
	ImGui::DragInt("Samples per pixel", samples, 4.0f, 1, 200);
	ImGui::End();
}


