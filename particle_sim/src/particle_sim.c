#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLFW/../../deps/linmath.h"

#include "vector_3d.h"
#include "mechanic_equations.h"

#define PI                  3.14159265358979323846264338327950f
#define CIRCLE_SEGMENTS     32


struct vertex
{
    float x, y;
    float r, g, b;
};


int file_to_text(FILE *fp, char **text);
void shader_compile_and_link(GLuint *program, const char *vs_text, const char *fs_text);
static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void draw_circle(const float cx, const float cy, const float r, const int num_segments, struct vertex *v);
static void render_loop(GLFWwindow *window, GLuint program, GLint mvp_location);

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";
const char vertex_shader_filename[32] = "shaders/vs.vert";
const char fragment_shader_filename[32] = "shaders/fs.frag";
struct vertex vertices[CIRCLE_SEGMENTS];
// static const struct vertex vertices[3] = {
//     { -0.6f, -0.4f, 1.f, 0.f, 0.f },
//     {  0.6f, -0.4f, 0.f, 1.f, 0.f },
//     {   0.f,  0.6f, 0.f, 0.f, 1.f }
// };


int main(void)
{
    const int initial_window_width = 640;
    const int initial_window_height = 480;
    GLuint VBO, program;
    GLint mvp_location, vpos_location, vcol_location;

    // char *vertex_shader_text = NULL;
    // char *fragment_shader_text = NULL;

    // FILE *vert_fp = fopen(vertex_shader_filename, "r");
    // FILE *frag_fp = fopen(fragment_shader_filename, "r");

    // file_to_text(vert_fp, &vertex_shader_text);
    // file_to_text(frag_fp, &fragment_shader_text);

    // fclose(vert_fp);
    // fclose(frag_fp);

    draw_circle(0.0f, 0.0f, 0.5f, CIRCLE_SEGMENTS, vertices);

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

    shader_compile_and_link(&program, vertex_shader_text, fragment_shader_text);

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
    // free(vertex_shader_text);
    // free(fragment_shader_text);

    return 0;
}


int file_to_text(FILE *fp, char **text)
{
    int retval = 1;

    fseek(fp, 0, SEEK_END);
    const unsigned int size = (unsigned int)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (!(*text=malloc(sizeof(char)*size)))
        retval = 0;
    else
        fread(*text, sizeof(char), size, fp);

    return retval;
}

void shader_compile_and_link(GLuint *program, const char *vs_text, const char *fs_text)
{
    GLuint vertex_shader, fragment_shader;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_text, NULL);
    glCompileShader(fragment_shader);
 
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);
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

static void draw_circle(const float cx, const float cy, const float r, const int num_segments, struct vertex *v)
{
    for(int i = 0; i < num_segments; ++i) {
        float theta = 2.0f * PI * i / (num_segments - 2);

        float x = r * cos(theta);
        float y = r * sin(theta);

        v[i].x = x + cx;
        v[i].y = y + cy;
        v[i].r = 1.0f;
        v[i].g = 1.0f;
        v[i].b = 1.0f;
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
        // mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, CIRCLE_SEGMENTS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
