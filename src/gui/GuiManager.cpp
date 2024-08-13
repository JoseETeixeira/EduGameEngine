// GUIManager.cpp
#include "GUIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <nfd.h> // Native File Dialog library (https://github.com/btzy/nativefiledialog-extended)
#include <filesystem>
#include <fstream>
#include <nanosvgrast.h>
#include <GL/gl.h>
#include <vector>
#include <iostream>
#include "../camera/camera.h"
#include <gtc/type_ptr.hpp>

// Icons
#include "icons/attachment_icon.h"
#include "icons/folder_icon.h"
#include "icons/jpg_icon.h"
#include "icons/mp4_icon.h"
#include "icons/png_icon.h"
#include "icons/text_icon.h"
#include "icons/arrow_up.h"
#include "icons/import_arrow.h"

GLuint folderIconTextureID;
GLuint textFileIconTextureID;
GLuint pngFileIconTextureID;
GLuint jpgFileIconTextureID;
GLuint attachmentIconTextureID;
GLuint arrowUpIconTextureID;
GLuint importArrowIconTextureID;

GLuint LoadTextureFromSVG(const std::string &svgData)
{
    // Parse the SVG from the provided data
    NSVGimage *image = nsvgParse((char *)svgData.c_str(), "px", 96.0f);
    if (!image)
    {
        std::cerr << "Failed to parse SVG data." << std::endl;
        return 0; // Return 0 on failure
    }

    // Create a rasterizer
    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (!rast)
    {
        std::cerr << "Failed to create rasterizer." << std::endl;
        nsvgDelete(image);
        return 0;
    }

    int width = (int)image->width;
    int height = (int)image->height;

    // Allocate memory for the bitmap
    std::vector<unsigned char> bitmap(width * height * 4); // 4 bytes per pixel (RGBA)

    // Rasterize the SVG into the bitmap
    nsvgRasterize(rast, image, 0, 0, 1, bitmap.data(), width, height, width * 4);

    // Clean up the rasterizer and the image after rasterization
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    // Generate an OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void GUIManager::InitializeIcons()
{
    folderIconTextureID = LoadTextureFromSVG(folder_icon_svg);
    textFileIconTextureID = LoadTextureFromSVG(text_icon_svg);
    pngFileIconTextureID = LoadTextureFromSVG(png_icon_svg);
    jpgFileIconTextureID = LoadTextureFromSVG(jpg_icon_svg);
    attachmentIconTextureID = LoadTextureFromSVG(attachment_icon_svg);
    arrowUpIconTextureID = LoadTextureFromSVG(arrow_up_icon_svg);
    importArrowIconTextureID = LoadTextureFromSVG(import_arrow_icon_svg);
}

GUIManager::GUIManager() : camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f)
{
    isPlaying = false;
}

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

    // Set the window creation flags (Make sure the window is resizable and has decorations)
    ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Remove the fullscreen setting to ensure buttons are visible
    int screenWidth, screenHeight;
    glfwGetWindowSize(window->GetGLFWWindow(), &screenWidth, &screenHeight);
    glfwSetWindowMonitor(window->GetGLFWWindow(), nullptr, 100, 100, screenWidth, screenHeight, 0);

    InitializeIcons();
    return true;
}

void GUIManager::NewFrame(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
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

void GUIManager::RenderEditorGUI(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
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
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 19));                             // Position it right below the menu bar
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40)); // Full width, height for buttons

    if (ImGui::Button("Play"))
    {
        isPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        isPlaying = false;
    }

    ImGui::End();

    // 3. Main area with resizable panels
    ImGui::SetNextWindowPos(ImVec2(0, 60));                                                            // Start below the buttons
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 60)); // Use full height below the controls
    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        // Workspace content, takes all the space except for the space used by the Sources panel
        ImGui::BeginChild("WorkspaceContent", ImVec2(0, ImGui::GetContentRegionAvail().y - 100), true); // Leaves space for Sources
        {
            ImGui::Columns(3, nullptr, true); // 3 columns with resizable splitters

            RenderOutlinePanel(); // Outline section
            ImGui::NextColumn();
            RenderMainEditorPanel(viewMatrix, projectionMatrix); // Main Editor section with tabs
            ImGui::NextColumn();
            RenderDetailsPanel(); // Details section

            ImGui::Columns(1); // End columns
        }
        ImGui::EndChild();

        // Render Sources as a resizable child of Workspace
        RenderSources();
    }
    ImGui::End();
}

void GUIManager::RenderOutlinePanel()
{
    ImGui::BeginChild("Outline", ImVec2(0, 0), true);
    {
        ImGui::Text("Outline");
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
    }
    ImGui::EndChild();
}

void GUIManager::RenderMainEditorPanel(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    ImGui::BeginChild("Main Editor", ImVec2(0, 0), true);
    {
        if (ImGui::BeginTabBar("MainEditorTabs"))
        {
            if (ImGui::BeginTabItem("Scene"))
            {
                // Ensure the grid is rendered within the ImGui window
                Render3DGrid(viewMatrix, projectionMatrix);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Game"))
            {
                if (isPlaying)
                {
                    RenderApplicationGUI(); // Render the Application GUI inside the Game tab
                }
                else
                {
                    ImGui::Text("Game view will appear here.");
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
}

void GUIManager::RenderDetailsPanel()
{
    ImGui::BeginChild("Details", ImVec2(0, 0), true);
    {
        ImGui::Text("Details");
        ImGui::Text("Transform:");
        // Add more component fields as necessary
    }
    ImGui::EndChild();
}

void GUIManager::RenderApplicationGUI()
{
    // Application GUI - This is where the game itself would be rendered
    ImGui::Text("Rendering the game inside the Game tab.");
    // Implement the game's rendering logic here
}

void GUIManager::Render3DGrid(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Use the provided view and projection matrices
    glm::mat4 view = viewMatrix;
    glm::mat4 projection = projectionMatrix;

    // Ensure the grid renders within the ImGui window
    ImGuizmo::SetDrawlist();

    // Set the ImGuizmo context to match the ImGui window
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    // Identity matrix for grid positioning
    static const float identityMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    // Draw the grid using ImGuizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::DrawGrid(glm::value_ptr(view), glm::value_ptr(projection), identityMatrix, 10.0f); // Adjust the grid size as needed
}

void GUIManager::ProcessInput()
{
    // Call camera.ProcessKeyboard and camera.ProcessMouseMovement based on user input
}

void GUIManager::Render()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::RenderSources()
{
    // Set transparent colors for the button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

    // Use a resizable child window
    ImGui::BeginChild("Sources", ImVec2(0, 0), true); // This allows vertical resizing
    {
        ImGui::Text("Sources");

        static std::string currentDirectory = std::filesystem::current_path().string();
        static std::vector<std::string> directoryStack;
        std::vector<std::filesystem::directory_entry> items;

        static float itemSize = 32.0f;

        for (const auto &entry : std::filesystem::directory_iterator(currentDirectory))
        {
            items.push_back(entry);
        }

        ImGui::Columns(1);

        if (!directoryStack.empty())
        {
            if (ImGui::ImageButton((void *)(intptr_t)arrowUpIconTextureID, ImVec2(16.0f, 16.0f), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0)))
            {
                currentDirectory = directoryStack.back();
                directoryStack.pop_back();
            }
        }

        if (ImGui::ImageButton((void *)(intptr_t)importArrowIconTextureID, ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0)))
        {
            ImportAsset();
        }

        if (ImGui::BeginPopupContextWindow("Sources Context Menu"))
        {
            if (ImGui::MenuItem("Create Folder"))
            {
                CreateFolder();
            }
            if (ImGui::MenuItem("New Player Controller"))
            {
                CreatePlayerController();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        int columns = 10;
        ImGui::Columns(columns, nullptr, false);

        for (const auto &item : items)
        {
            GLuint iconTextureID = attachmentIconTextureID;

            if (item.is_directory())
            {
                iconTextureID = folderIconTextureID;
            }
            else
            {
                std::string extension = item.path().extension().string();
                if (extension == ".txt")
                {
                    iconTextureID = textFileIconTextureID;
                }
                else if (extension == ".png")
                {
                    iconTextureID = pngFileIconTextureID;
                }
                else if (extension == ".jpg" || extension == ".jpeg")
                {
                    iconTextureID = jpgFileIconTextureID;
                }
            }

            ImGui::Image((void *)(intptr_t)iconTextureID, ImVec2(itemSize, itemSize));

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (item.is_directory())
                {
                    directoryStack.push_back(currentDirectory);
                    currentDirectory = item.path().string();
                }
                else
                {
                    selectedAsset = item.path().string();
                }
            }

            ImGui::TextWrapped(item.path().filename().string().c_str());
            ImGui::NextColumn();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(3);
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

// Create a new folder in the current working directory
void GUIManager::CreateFolder()
{
    std::string folderName = "New Folder"; // Placeholder name
    std::filesystem::create_directory(std::filesystem::current_path() / folderName);
}

// Create a new player controller file
void GUIManager::CreatePlayerController()
{
    std::string fileName = "PlayerController.cpp";
    std::ofstream file(fileName);
    file << "// Player Controller Code" << std::endl;
    file.close();
}
