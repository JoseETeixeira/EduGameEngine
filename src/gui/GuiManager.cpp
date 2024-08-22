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
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include "../ecs/MovementSystem.h"
#include "../ecs/NameComponent.h"
#include <json.hpp>

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

void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    GUIManager *guiManager = static_cast<GUIManager *>(glfwGetWindowUserPointer(window));
    guiManager->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

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

GUIManager::GUIManager()
    : camera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f), firstMouse(true), lastX(400), lastY(300), currentTransformMode(TransformMode::TRANSLATE), objectSelected(false)
{
    isPlaying = false;
    // Adjust the camera's initial position and orientation to better view the grid
    camera.Position = glm::vec3(0.0f, 5.0f, 15.0f); // Move the camera further back
    camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    contentDrawerOpen = false;
}

GUIManager::~GUIManager()
{
    delete scene; // Clean up the scen
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUIManager::ApplyTransformation(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    if (selectedEntity != entt::null) // Check if an entity is selected
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

        ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
        if (currentTransformMode == TransformMode::TRANSLATE)
            operation = ImGuizmo::TRANSLATE;
        else if (currentTransformMode == TransformMode::ROTATE)
            operation = ImGuizmo::ROTATE;
        else if (currentTransformMode == TransformMode::SCALE)
            operation = ImGuizmo::SCALE;

        // Accessing the TransformComponent from the registry
        if (registry->valid(selectedEntity))
        {
            auto &transform = registry->get<TransformComponent>(selectedEntity);
            selectedObjectTransform = glm::translate(glm::mat4(1.0f), transform.position) *
                                      glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
                                      glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                      glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
                                      glm::scale(glm::mat4(1.0f), transform.scale);

            ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
                                 operation, ImGuizmo::LOCAL, glm::value_ptr(selectedObjectTransform));

            // Update the entity's transform component
            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selectedObjectTransform),
                                                  glm::value_ptr(translation),
                                                  glm::value_ptr(rotation),
                                                  glm::value_ptr(scale));
            transform.position = translation;
            transform.rotation = rotation;
            transform.scale = scale;
        }
    }
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

    // Register the scroll callback
    glfwSetScrollCallback(window->GetGLFWWindow(), ScrollCallback);

    // Set the user pointer to this instance of GUIManager
    glfwSetWindowUserPointer(window->GetGLFWWindow(), this);

    InitializeIcons();
    return true;
}

void GUIManager::NewFrame(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &screenWidth, &screenHeight);
    float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

    // Set the viewport to cover the entire window
    glGetIntegerv(GL_VIEWPORT, viewport);

    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);

    // Correct the near and far plane values
    // glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Editor GUI - Interface for the game engine
    RenderEditorGUI(viewMatrix, projectionMatrix);
    glViewport(viewport[0], viewport[1], screenWidth, screenHeight);
    glScissor(scissorBox[0], scissorBox[1], screenWidth, screenHeight);
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
            if (ImGui::MenuItem("New Project"))
            {
                CreateNewProject();
            }
            if (ImGui::MenuItem("Open Project"))
            {
                OpenProject();
            }
            if (ImGui::MenuItem("Save Project"))
            {
                SaveProject();
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
        engine->SetPlaying(true); // Call SetPlaying with true
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        engine->SetPlaying(false);
    }

    ImGui::End();

    // 3. Main area with resizable panels
    float remainingHeight = ImGui::GetIO().DisplaySize.y - 100; // Deducting height for controls and content drawer button

    ImGui::SetNextWindowPos(ImVec2(0, 60));                                          // Start below the buttons
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, remainingHeight)); // Use full height below the controls
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));                    // RGBA with A=0 for transparency

    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        // Workspace content, takes all the space except for the space used by the Sources panel
        ImGui::BeginChild("WorkspaceContent", ImVec2(0, remainingHeight - 40), true); // Leaves space for the Content Drawer button
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

        // Content Drawer Button
        if (!contentDrawerOpen)
        {
            ImGui::SetCursorPosY(remainingHeight - 30); // Adjust the position to the bottom
            if (ImGui::Button("Open Drawer", ImVec2(ImGui::GetIO().DisplaySize.x / 5, 30)))
            {
                contentDrawerOpen = !contentDrawerOpen; // Toggle the drawer open state
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    // 4. Force render the Sources panel at a fixed position for testing
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 300));    // Fixed position at the bottom
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 300));       // Fixed height for the drawer
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f)); // Dark semi-transparent background for the drawer

    if (contentDrawerOpen)
    {
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 300));    // Position at the bottom
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 300));       // Fixed height for the drawer
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f)); // Dark semi-transparent background for the drawer

        ImGui::Begin("SourcesDrawer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse);
        {
            if (ImGui::Button("Close Drawer", ImVec2(ImGui::GetIO().DisplaySize.x / 5, 30)))
            {
                contentDrawerOpen = !contentDrawerOpen; // Toggle the drawer open state
            }
            RenderSources(); // Render the Sources content here
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();
}

// Function to create a new project
void GUIManager::CreateNewProject()
{
    nfdu8char_t *outPath = NULL;
    nfdresult_t result = NFD_PickFolderU8(&outPath, NULL);
    if (result == NFD_OKAY)
    {
        std::string projectDir(outPath);
        std::string projectFilePath = projectDir + "/project.ege";
        std::free(outPath);

        // Create a new .ege file with default content
        std::ofstream file(projectFilePath);
        file << R"({
            "project_name": "NewEduGameProject",
            "version": "1.0",
            "main_scene": "scenes/main.escn",
            "scenes": ["scenes/main.escn"],
            "plugins": [],
            "dependencies": {}
        })";
        file.close();

        // Notify the user
        ImGui::OpenPopup("Project Created");

        // You can create the scenes directory and main scene file here as well
        std::filesystem::create_directory(projectDir + "/scenes");
        std::ofstream mainSceneFile(projectDir + "/scenes/main.escn");
        mainSceneFile << R"({
            "scene_name": "MainScene",
            "objects": [],
            "sub_scenes": [],
            "scripts": []
        })";
        mainSceneFile.close();
    }
    else if (result == NFD_CANCEL)
    {
        std::cout << "User canceled." << std::endl;
    }
    else
    {
        std::cerr << "Error: " << NFD_GetError() << std::endl;
    }
}

// Function to open an existing project
void GUIManager::OpenProject()
{
    nfdu8char_t *outPath = NULL;
    const nfdu8filteritem_t filterItem[1] = {{"EduGameEngine Project", "ege"}};
    nfdresult_t result = NFD_OpenDialogU8(&outPath, filterItem, 1, NULL);

    if (result == NFD_OKAY)
    {
        std::string projectFilePath(outPath);
        std::free(outPath);

        // Load the .ege file content
        std::ifstream file(projectFilePath);
        if (file.is_open())
        {
            nlohmann::json projectJson;
            file >> projectJson;
            file.close();

            // Open the main scene specified in the .ege file
            std::string mainScenePath = projectJson["main_scene"];
            LoadScene(mainScenePath); // Implement this function to load the scene

            // Notify the user
            ImGui::OpenPopup("Project Loaded");
        }
        else
        {
            std::cerr << "Failed to open project file." << std::endl;
        }
    }
    else if (result == NFD_CANCEL)
    {
        std::cout << "User canceled." << std::endl;
    }
    else
    {
        std::cerr << "Error: " << NFD_GetError() << std::endl;
    }
}

// Function to save the current project
void GUIManager::SaveProject()
{
    // Implement saving the current project state back to the .ege file
    // This could involve writing any unsaved changes to scenes, objects, etc.
}

// Function to load a scene from a file path
void GUIManager::LoadScene(const std::string &scenePath)
{
    // Implement logic to load the scene from the .escn file
    // This would include reading the file and creating objects, components, etc.
}

void GUIManager::RenderOutlinePanel()
{
    ImGui::BeginChild("Outline", ImVec2(0, 0), true);
    {
        ImGui::Text("Outline");

        // Render all entities in the scene
        registry->view<TransformComponent>().each([&](auto entity, TransformComponent &transform)
                                                  {
            if (transform.parent == entt::null) // Check if the entity is at the root
            {
                RenderEntityNode(entity, transform);
            } });
    }
    ImGui::EndChild();
}

void GUIManager::RenderEntityNode(entt::entity entity, TransformComponent &transform)
{
    // Ensure NameComponent exists before accessing it
    if (registry->all_of<NameComponent>(entity))
    {
        auto &name = registry->get<NameComponent>(entity).name;

        ImGuiTreeNodeFlags flags = ((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void *)(intptr_t)entity, flags, name.c_str());

        if (ImGui::IsItemClicked())
        {
            selectedEntity = entity;
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("REORDER_ENTITY", &entity, sizeof(entt::entity));
            ImGui::Text("Reparent %s", name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("REORDER_ENTITY"))
            {
                entt::entity droppedEntity = *(const entt::entity *)payload->Data;
                registry->get<TransformComponent>(droppedEntity).parent = entity;
            }
            ImGui::EndDragDropTarget();
        }

        if (opened)
        {
            // Render children
            registry->view<TransformComponent>().each([&](auto childEntity, TransformComponent &childTransform)
                                                      {
                if (childTransform.parent == entity) {
                    RenderEntityNode(childEntity, childTransform);
                } });

            ImGui::TreePop();
        }
    }
}
void GUIManager::RenderMainEditorPanel(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    ImGui::BeginChild("Main Editor", ImVec2(0, 0), true);
    {
        if (ImGui::BeginTabBar("MainEditorTabs"))
        {
            if (ImGui::BeginTabItem("Scene"))
            {
                if (ImGui::Button("Translate"))
                {
                    currentTransformMode = TransformMode::TRANSLATE;
                }
                ImGui::SameLine();
                if (ImGui::Button("Rotate"))
                {
                    currentTransformMode = TransformMode::ROTATE;
                }
                ImGui::SameLine();
                if (ImGui::Button("Scale"))
                {
                    currentTransformMode = TransformMode::SCALE;
                } // Render the transform buttons

                // Ensure the grid is rendered within the ImGui window
                Render3DGrid(viewMatrix, projectionMatrix);

                // Debug: Check if the scene is being rendered
                std::cout << "Rendering Scene" << std::endl;

                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 windowSize = ImGui::GetWindowSize();
                // Set the viewport to the current ImGui window size and position
                glViewport(static_cast<GLsizei>(windowPos.x), static_cast<GLsizei>(windowPos.y),
                           static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));

                // Set the scissor box to match the current ImGui window
                glScissor(static_cast<GLsizei>(windowPos.x), static_cast<GLsizei>(windowPos.y),
                          static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));

                // Render the scene
                renderer->Render(scene, viewMatrix, projectionMatrix);

                // Apply transformations to the selected object
                ApplyTransformation(viewMatrix, projectionMatrix);

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
    ImGuizmo::SetDrawlist();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    glm::mat4 identity = glm::mat4(1.0f);

    ImGuizmo::BeginFrame();
    glDisable(GL_DEPTH_TEST);
    ImGuizmo::DrawGrid(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), glm::value_ptr(identity), 100.0f);
    glEnable(GL_DEPTH_TEST);
}

void GUIManager::ProcessInput(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void GUIManager::Render(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Clear the framebuffer before drawing anything
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the 3D content (like the red cube)
    NewFrame(viewMatrix, projectionMatrix); // This will call RenderEditorGUI, which renders the cube

    // Render the ImGui interface
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers (typically done in the main loop, not here)
}

void GUIManager::RenderSources()
{
    // Set transparent colors for the button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

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

    ImGui::PopStyleColor(3);
}

void GUIManager::ImportAsset()
{
    nfdu8char_t *outPath = NULL;
    const nfdu8filteritem_t filterItem[3] = {
        {"Image Files", "png,jpg"},
        {"Object Files", "obj"},
        {"3D Model Files", "obj,fbx"}};

    nfdresult_t result = NFD_OpenDialogU8(&outPath, filterItem, 3, NULL); // Include .fbx filter
    if (result == NFD_OKAY)
    {
        std::string assetPath(reinterpret_cast<char *>(outPath));
        free(outPath); // Free memory allocated by NFD

        AddAssetToScene(outPath);
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
    // Use Assimp to load the model
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Error::Assimp:: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Process the Assimp scene and add it to our Scene class
    this->scene->ProcessAssimpScene(scene);

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
