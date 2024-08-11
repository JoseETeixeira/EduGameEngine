#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Engine
{
public:
    GLFWwindow *window;

    bool Initialize()
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Set the OpenGL version to 3.3 (you can adjust this depending on your needs)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core Profile

        // Create a windowed mode window and its OpenGL context
        window = glfwCreateWindow(800, 600, "My Game Engine", nullptr, nullptr);
        if (!window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        // Make the window's context current
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }

        // Set the viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Set the clear color to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        return true;
    }

    void Run()
    {
        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            // Render here
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    void Shutdown()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main()
{
    Engine engine;

    if (!engine.Initialize())
    {
        return -1;
    }

    engine.Run();
    engine.Shutdown();

    return 0;
}
