#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector_3d.h"
#include "mechanic_equations.h"


static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 1.f, 1.f },
    {  0.6f, -0.4f, 1.f, 1.f, 1.f },
    {   0.f,  0.6f, 1.f, 1.f, 1.f }
};

static char *vertex_shader_text = NULL;
static char *fragment_shader_text = NULL;


int file_to_text(FILE *fp, char **text);
void error_callback(int error, const char *description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void draw_circle(float cx, float cy, float r, int num_segments, mat4x4 *m);
void render_loop(GLFWwindow* window, GLuint program, GLint mvp_location);

int main(void)
{
    GLuint VBO, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    FILE *vert_fp = fopen("shaders/vs.vert", "r");
    FILE *frag_fp = fopen("shaders/fs.frag", "r");

    file_to_text(vert_fp, &vertex_shader_text);
    file_to_text(frag_fp, &fragment_shader_text);

    fclose(vert_fp);
    fclose(frag_fp);

    glfwSetErrorCallback(error_callback);
    assert(glfwInit());

    GLFWwindow* window = glfwCreateWindow(640, 480, "Particle Sim", NULL, NULL);
    assert(window);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar * const*)&vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar * const*)&fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    render_loop(window, program, mvp_location);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    free(vertex_shader_text);
    free(fragment_shader_text);

    return 0;
}


int file_to_text(FILE *fp, char **text)
{
    int retval = 1;

    fseek(fp, 0, SEEK_END);
    const unsigned int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (!(*text=malloc(sizeof(char)*size)))
        retval = 0;
    else {
        fread(*text, sizeof(char), size, fp);
    }

    return retval;
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

void draw_circle(float cx, float cy, float r, int num_segments, mat4x4 *m)
{
    vec2 v;

    for(int i = 0; i < num_segments; ++i) {
        float theta = 2.0f * 3.1415926f * (float)i / (float)num_segments; //get the current angle

        float x = r * cosf(theta); //calculate the x component
        float y = r * sinf(theta); //calculate the y component

        v[0] = x + cx;
        v[1] = y + cy;
    }
}

void render_loop(GLFWwindow* window, GLuint program, GLint mvp_location)
{
    while (!glfwWindowShouldClose(window)) {
        
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
