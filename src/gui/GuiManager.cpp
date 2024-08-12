// GUIManager.cpp
#include "GUIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <nfd.h> // Native File Dialog library (https://github.com/btzy/nativefiledialog-extended)
#include <iostream>
#include <filesystem>
#include <fstream>

GUIManager::GUIManager() : isPlaying(false) {}

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

    // Set the window to take up the whole screen in windowed mode
    int screenWidth, screenHeight;
    glfwGetWindowSize(window->GetGLFWWindow(), &screenWidth, &screenHeight);
    glfwSetWindowMonitor(window->GetGLFWWindow(), nullptr, 0, 0, screenWidth, screenHeight, 0);

    return true;
}

void GUIManager::NewFrame(const float *viewMatrix, const float *projectionMatrix)
{
    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Editor GUI - Interface for the game engine
    RenderEditorGUI(viewMatrix, projectionMatrix);

    // Render ImGui
    ImGui::Render();
}

void GUIManager::RenderEditorGUI(const float *viewMatrix, const float *projectionMatrix)
{
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
        // Start play mode
        isPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        // Stop play mode
        isPlaying = false;
    }

    ImGui::End();

    // 3. Divide the main area into three sections: Outline, Main Editor, and Details
    ImGui::SetNextWindowPos(ImVec2(0, 60)); // Start below the buttons
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 160));
    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();
        float columnWidth = ContentRegionAvail.x * 0.25f;

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
            if (isPlaying)
            {
                RenderApplicationGUI(); // Render the Application GUI inside the Main Editor
            }
            else
            {
                ImGui::Text("Main Editor");
                // Insert game editor content here (e.g., rendering game scene, handling input, etc.)
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Details section
        ImGui::BeginChild("Details", ImVec2(ContentRegionAvail.x * 0.25f, ContentRegionAvail.y), true);
        {
            ImGui::Text("Details");
            // Show details of selected nodes (e.g., position, components, etc.)
            ImGui::Text("Transform:");
            // Add more component fields as necessary
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // 4. Sources section
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 100)); // Position at the bottom
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 100));    // Take up the full width
    ImGui::Begin("Sources", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImGui::Text("Sources");

        // Example of displaying and interacting with the filesystem
        static std::string currentDirectory = std::filesystem::current_path().string();

        ImGui::Text("Current Directory: %s", currentDirectory.c_str());
        ImGui::Separator();

        // Listing the directory contents
        for (const auto &entry : std::filesystem::directory_iterator(currentDirectory))
        {
            if (entry.is_directory())
            {
                if (ImGui::TreeNode(entry.path().filename().string().c_str()))
                {
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::Text("%s", entry.path().filename().string().c_str());
            }
        }

        ImGui::Separator();

        // Right-click context menu to create folders or import files
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("New Folder"))
            {
                std::filesystem::create_directory(currentDirectory + "/New Folder");
            }

            if (ImGui::MenuItem("Import Asset"))
            {
                ImportAsset(); // Handle importing an asset
            }

            if (ImGui::MenuItem("Create Player Controller"))
            {
                std::ofstream file(currentDirectory + "/PlayerController.cpp");
                file << "// Player Controller Implementation";
                file.close();
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();

    // Example of manipulating a matrix using ImGuizmo
    ImGuizmo::BeginFrame();
    static float matrix[16] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f};

    if (!isPlaying)
    {
        ImGuizmo::Manipulate(viewMatrix, projectionMatrix, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, matrix);
    }
}

void GUIManager::RenderApplicationGUI()
{
    // Application GUI - This is where the game itself would be rendered
    ImGui::Text("Rendering the game inside the Main Editor.");
    // Implement the game's rendering logic here
}

void GUIManager::Render()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::ImportAsset()
{
    nfdu8char_t *outPath = NULL;
    const nfdu8filteritem_t filterItem[3] = {
        {"Image Files", "png,jpg"},
        {"Object Files", "obj"},
    };

    nfdresult_t result = NFD_OpenDialogU8(&outPath, filterItem, 2, NULL); // Filter for relevant asset types
    if (result == NFD_OKAY)
    {
        std::string assetPath(reinterpret_cast<char *>(outPath));
        free(outPath); // Free memory allocated by NFD

        // Implement your logic to import and handle the asset
        AddAssetToScene(assetPath);
    }
    else if (result == NFD_CANCEL)
    {
        std::cout << "User canceled file dialog." << std::endl;
    }
    else
    {
        std::cerr << "Error: " << NFD_GetError() << std::endl;
    }
}

void GUIManager::AddAssetToScene(const std::string &assetPath)
{
    // Implement the logic to add the asset to the scene or outline
    // For example, create a new node in the outline and attach the asset to it
    std::cout << "Asset imported: " << assetPath << std::endl;
}
