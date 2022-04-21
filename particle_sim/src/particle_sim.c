#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>

#include "vector_3d.h"
#include "mechanic_equations.h"


void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_loop(GLFWwindow* window);

int main(void)
{
    glfwSetErrorCallback(error_callback);
    assert(glfwInit());

    GLFWwindow* window = glfwCreateWindow(640, 480, "Particle Sim", NULL, NULL);
    assert(window);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    window_loop(window);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void window_loop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        
        float ratio;
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
