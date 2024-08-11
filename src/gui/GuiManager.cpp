// GUIManager.cpp
#include "GUIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

GUIManager::GUIManager() {}

GUIManager::~GUIManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool GUIManager::Initialize(Window *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void GUIManager::NewFrame(const float *viewMatrix, const float *projectionMatrix)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Begin ImGui frame
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some text in a window.");
    ImGui::End();

    ImGuizmo::BeginFrame();

    // Example of manipulating a matrix using ImGuizmo
    static float matrix[16] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f};

    ImGuizmo::Manipulate(viewMatrix, projectionMatrix, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, matrix);
}

void GUIManager::Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
