#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(int width, int height, const char *title, bool borderless = false);
    ~Window();

    bool Initialize();
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;

    GLFWwindow *GetGLFWWindow() const;

private:
    int width, height;
    const char *title;
    GLFWwindow *window;
    bool borderless;
};
