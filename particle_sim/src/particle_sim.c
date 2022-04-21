#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>

#include "vector_3d.h"
#include "mechanic_equations.h"


void error_callback(int error, const char* description);
void window_loop(GLFWwindow* window);

int main(void)
{
    assert(glfwInit());

    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    assert(window);

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

void window_loop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        
    }
}
