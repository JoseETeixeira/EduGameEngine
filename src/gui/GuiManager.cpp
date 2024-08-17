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

// Function to compile shaders
GLuint CompileShader(const char *source, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}

// Function to create a shader program
GLuint CreateShaderProgram()
{
    // Vertex Shader source code
    const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    )";

    // Fragment Shader source code
    // Fragment Shader source code
    const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Set the fragment color to bright red
    }
    )";

    // Compile vertex shader
    GLuint vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);

    // Compile fragment shader
    GLuint fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Link shaders into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    // Clean up shaders as they are now linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

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
    : camera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f), firstMouse(true), lastX(400), lastY(300)
{
    isPlaying = false;
    // Adjust the camera's initial position and orientation to better view the grid
    camera.Position = glm::vec3(0.0f, 5.0f, 15.0f); // Move the camera further back
    camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
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

    // Correct the near and far plane values
    // glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

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
        engine->SetPlaying(true); // Call SetPlaying with true
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        engine->SetPlaying(false);
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

                // Render the test cube within the Scene tab
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 windowSize = ImGui::GetWindowSize();

                // Set the viewport to match the Scene tab size
                glViewport(static_cast<GLsizei>(windowPos.x), static_cast<GLsizei>(windowPos.y),
                           static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));

                // Render the cube after setting the viewport
                RenderTestCube(viewMatrix, projectionMatrix);

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

void GUIManager::RenderTestCube(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    static auto cubeEntity = engine->registry.create();
    static bool initialized = false;

    if (!initialized)
    {
        engine->registry.emplace<PositionComponent>(cubeEntity, glm::vec3(0.0f, 0.0f, 0.0f));
        engine->registry.emplace<VelocityComponent>(cubeEntity, glm::vec3(1.0f, 1.0f, 0.0f));
        initialized = true;
    }

    // Retrieve the position from the PositionComponent
    auto &position = engine->registry.get<PositionComponent>(cubeEntity);

    // Set the viewport to cover the entire window
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    glScissor(0, 0, screenWidth, screenHeight);
    glEnable(GL_SCISSOR_TEST);

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Disable face culling to check visibility issues
    glDisable(GL_CULL_FACE);

    // Create and use the shader program
    GLuint shaderProgram = CreateShaderProgram();
    glUseProgram(shaderProgram);

    // Define the cube vertices
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f};

    // Generate and bind VAO and VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Set the uniforms for the shader
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // Update the model matrix with the current position
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position.position);
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the cube
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Unbind the VAO
    glBindVertexArray(0);

    // Disable depth testing after rendering
    glDisable(GL_DEPTH_TEST);

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cout << "OpenGL Error: " << error << std::endl;
    }
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
