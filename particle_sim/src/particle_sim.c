#include <stdio.h>
#include <assert.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "vector_3d.h"
#include "mechanic_equations.h"


void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void render_loop(GLFWwindow* window);

int main(void)
{
    glfwSetErrorCallback(error_callback);
    assert(glfwInit());

    GLFWwindow* window = glfwCreateWindow(640, 480, "Particle Sim", NULL, NULL);
    assert(window);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    render_loop(window);
    
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

void render_loop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
