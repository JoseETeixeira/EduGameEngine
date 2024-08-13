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

// Icons
#include "icons/attachment_icon.h"
#include "icons/folder_icon.h"
#include "icons/jpg_icon.h"
#include "icons/mp4_icon.h"
#include "icons/png_icon.h"
#include "icons/text_icon.h"

GLuint folderIconTextureID;
GLuint textFileIconTextureID;
GLuint pngFileIconTextureID;
GLuint jpgFileIconTextureID;
GLuint attachmentIconTextureID;

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
}

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

    InitializeIcons();
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

    // Sources section
    RenderSources();

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

void GUIManager::RenderSources()
{
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 100)); // Position at the bottom
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 100));    // Take up the full width
    ImGui::Begin("Sources", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImGui::Text("Sources");

        // Variables for directory handling
        static std::string currentDirectory = std::filesystem::current_path().string();
        static std::vector<std::string> directoryStack;
        std::vector<std::filesystem::directory_entry> items;

        // Reading the current directory
        for (const auto &entry : std::filesystem::directory_iterator(currentDirectory))
        {
            items.push_back(entry);
        }

        ImGui::Columns(1);

        if (!directoryStack.empty() && ImGui::Button("Up"))
        {
            // Navigate up one directory level
            currentDirectory = directoryStack.back();
            directoryStack.pop_back();
        }

        if (ImGui::Button("Import Asset"))
        {
            ImportAsset(); // Call the method to import an asset
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
        int columns = 10; // Number of columns
        ImGui::Columns(columns, nullptr, false);

        for (const auto &item : items)
        {
            GLuint iconTextureID = attachmentIconTextureID; // Default to attachment icon

            if (item.is_directory())
            {
                iconTextureID = folderIconTextureID; // Use folder icon
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

            // Render the icon
            ImGui::Image((void *)(intptr_t)iconTextureID, ImVec2(50, 50));

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (item.is_directory())
                {
                    // Navigate into the directory
                    directoryStack.push_back(currentDirectory);
                    currentDirectory = item.path().string();
                }
                else
                {
                    // Handle file selection
                    selectedAsset = item.path().string(); // This should be valid now
                }
            }

            ImGui::TextWrapped(item.path().filename().string().c_str());
            ImGui::NextColumn();
        }
    }
    ImGui::End();
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