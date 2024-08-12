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
    // 1. Create the Menu Bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            { /* Handle new action */
            }
            if (ImGui::MenuItem("Open"))
            { /* Handle open action */
            }
            if (ImGui::MenuItem("Save"))
            { /* Handle save action */
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo"))
            { /* Handle undo action */
            }
            if (ImGui::MenuItem("Redo"))
            { /* Handle redo action */
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Settings"))
            { /* Handle settings action */
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // 2. Create the Play and Stop buttons below the menu bar
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(ImVec2(0, 19));                             // Position it right below the menu bar
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40)); // Full width, height for buttons

    if (ImGui::Button("Play"))
    {
        // Handle play action
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        // Handle stop action
    }

    ImGui::End();

    // 3. Divide the main area into three sections: Outline, Main Editor, and Details
    ImGui::SetNextWindowPos(ImVec2(0, 60)); // Start below the buttons
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 60));
    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();
        float columnWidth = ContentRegionAvail.x * 0.3f;

        // Outline section
        ImGui::BeginChild("Outline", ImVec2(columnWidth, ContentRegionAvail.y), true);
        {
            ImGui::Text("Outline");
            // Add nodes, etc.
            if (ImGui::TreeNode("Node1"))
            {
                ImGui::Text("Node1 Child");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Node2"))
            {
                ImGui::Text("Node2 Child");
                ImGui::TreePop();
            }
            // You can add options to add/remove nodes
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Main Editor section
        ImGui::BeginChild("Main Editor", ImVec2(ContentRegionAvail.x * 0.5f, ContentRegionAvail.y), true);
        {
            ImGui::Text("Main Editor");
            // Insert game editor content here (e.g., rendering game scene, handling input, etc.)
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Details section
        ImGui::BeginChild("Details", ImVec2(ContentRegionAvail.x * 0.2f, ContentRegionAvail.y), true);
        {
            ImGui::Text("Details");
            // Show details of selected nodes (e.g., position, components, etc.)
            ImGui::Text("Transform:");

            // Add more component fields as necessary
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // Example of manipulating a matrix using ImGuizmo
    ImGuizmo::BeginFrame();
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
