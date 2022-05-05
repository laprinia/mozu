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

void GUIManager::DrawSampleData(int* samples, int* maxDepth, bool* hasBloom, float* threshold, float* strenght, float* radius)
{
	ImGui::Begin("mozu");
	ImGui::DragInt("Samples per pixel", samples, 4.0f, 1, 200);
	ImGui::DragInt("Max Depth", maxDepth, 10.0f, 5, 200);
	ImGui::Checkbox("Bloom Effect", hasBloom);
	ImGui::DragFloat("Bloom Threshold", threshold, 0.1f, *threshold, 10.0);
	ImGui::DragFloat("Bloom Strength", strenght, 1.0f, *strenght, 40.0);
	ImGui::DragFloat("Bloom Radius", radius, 1.0f, *radius, 40.0);
	ImGui::End();
}


