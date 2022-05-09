#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <malloc.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLFW/../../deps/linmath.h"

#include "vector.h"
#include "mechanic_equations.h"

#define PI                  3.14159265358979323846264338327950f
#define CIRCLE_SEGMENTS     32


struct vertex
{
    float x, y;
    color_t color;
};


static size_t get_file_size(FILE *fp);
static void shader_compile_and_link(GLuint *program);
static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void draw_circle(const float cx, const float cy, const float r, const int num_segments, const color_t color, struct vertex *v);
static void render_loop(GLFWwindow *window, GLuint program, GLint mvp_location);


const char vertex_shader_filepath[] = "shaders/vs.vert";
const char fragment_shader_filepath[] = "shaders/fs.frag";


int main(void)
{
    const int initial_window_width = 640;
    const int initial_window_height = 480;
    const color_t color = {1.0f, 1.0f, 1.0f};
    struct vertex vertices[CIRCLE_SEGMENTS];
    GLuint VBO, program;
    GLint mvp_location, vpos_location, vcol_location;

    draw_circle(0.0f, 0.0f, 0.5f, CIRCLE_SEGMENTS, color, vertices);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        glfwTerminate();
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(initial_window_width, initial_window_height, "Particle Sim", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    shader_compile_and_link(&program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
 
    glEnableVertexAttribArray((GLuint)vpos_location);
    glVertexAttribPointer((GLuint)vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray((GLuint)vcol_location);
    glVertexAttribPointer((GLuint)vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    render_loop(window, program, mvp_location);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


static size_t get_file_size(FILE *fp)
{
    size_t size;

    fseek(fp, 0, SEEK_END);
    size = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return size;
}

static void shader_compile_and_link(GLuint *program)
{
    GLuint vertex_shader, fragment_shader;
    char vs_text[256];
    char fs_text[256];
    FILE *vs_fp = fopen(vertex_shader_filepath, "r");
    FILE *fs_fp = fopen(fragment_shader_filepath, "r");

    if (!vs_fp || !fs_fp)
        exit(ERROR_FILE_NOT_FOUND);

    const size_t vs_size = get_file_size(vs_fp);
    const size_t fs_size = get_file_size(fs_fp);

    // vs_text = _alinged_malloc(sizeof(char)*vs_size, sizeof(char));
    // fs_text = _alinged_malloc(sizeof(char)*fs_size, sizeof(char));

    // if (!vs_text || !fs_text)
    //     exit(ERROR_NOT_ENOUGH_MEMORY);

    if (fread(vs_text, sizeof(char), vs_size, vs_fp) != vs_size)
        exit(ERROR_INVALID_DATA);
    if (fread(fs_text, sizeof(char), fs_size, fs_fp) != fs_size)
        exit(ERROR_INVALID_DATA);

    fclose(vs_fp);
    fclose(fs_fp);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar * const*)&vs_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar * const*)&fs_text, NULL);
    glCompileShader(fragment_shader);
 
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);

    // _alinged_free(vs_text);
    // _alinged_free(fs_text);
}

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void draw_circle(const float cx, const float cy, const float r, const int num_segments, const color_t color, struct vertex *v)
{
    for(int i = 0; i < num_segments; ++i) {
        float theta = 2.0f * PI * i / (num_segments - 2);

        float x = r * cos(theta);
        float y = r * sin(theta);

        v[i].x = x + cx;
        v[i].y = y + cy;
        v[i].color = color;
    }
}

static void render_loop(GLFWwindow *window, GLuint program, GLint mvp_location)
{
    while (!glfwWindowShouldClose(window)) {
        
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float)width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
