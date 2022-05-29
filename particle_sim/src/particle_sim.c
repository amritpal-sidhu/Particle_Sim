#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector.h"
#include "mechanic_equations.h"

#define PI                  3.14159265358979323846264338327950f
#define CIRCLE_SEGMENTS     32
#define P_COUNT             1
#define E_COUNT             2


struct vertex
{
    float x, y;
    color_t color;
};


static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static size_t get_file_size(FILE *fp);
static void shader_compile_and_link(GLuint *program);

static void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size);
static void vertex_buffer_draw(const GLuint VBO, const float ratio, const float x, const float y, const float z);
static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO);

static void draw_circle(const float cx, const float cy, const float r, const int num_segments, const color_t color, struct vertex *v);
static void busy_wait_ms(const float delay_in_ms);


/**
 * Local global variables
 *   TODO: Use parameter passing once the code becomes
 *         more stable.
 */
static const char vertex_shader_filepath[] = "shaders/vs.vert";
static const char fragment_shader_filepath[] = "shaders/fs.frag";
static const char debug_output_filepath[] = "debug_output.txt";
static FILE *debug_fp;

static GLint vpos_location, vcol_location, mvp_location;
static mat4x4 v;


/* Entry point */
int main(void)
{
    const int initial_window_width = 640;
    const int initial_window_height = 480;
    
    const float p_radius = 0.1f;
    const float e_radius = 0.05f;
    const color_t p_color = {1.0f, 0.0f, 0.0f};
    const color_t e_color = {0.0f, 0.0f, 1.0f};
    struct vertex p_vertices[CIRCLE_SEGMENTS];
    struct vertex e_vertices[CIRCLE_SEGMENTS];
    GLuint VBO[P_COUNT+E_COUNT], program;

    printf("Attempting to open debug file\n");
    debug_fp = fopen(debug_output_filepath, "w");
    if (!debug_fp) {
        printf("failed to open debug file\n");
        return 1;
    }

    fprintf(debug_fp, "*** DEBUG OUTPUT LOG ***\n\n");

    draw_circle(0.0f, 0.0f, p_radius, CIRCLE_SEGMENTS, p_color, p_vertices);
    draw_circle(0.0f, 0.0f, e_radius, CIRCLE_SEGMENTS, e_color, e_vertices);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        glfwTerminate();
        fclose(debug_fp);
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(initial_window_width, initial_window_height, "Particle Sim", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fclose(debug_fp);
        return 1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    
    shader_compile_and_link(&program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    for (size_t i = 0; i < P_COUNT; ++i)
        vertex_buffer_init(&VBO[i], p_vertices, sizeof(p_vertices));

    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        vertex_buffer_init(&VBO[i], e_vertices, sizeof(e_vertices));
 
    render_loop(window, program, VBO);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    fprintf(debug_fp, "Program terminated correctly.\n");
    fclose(debug_fp);

    return 0;
}


/* Local function definitions */
static void error_callback(int error, const char *description)
{
    fprintf(debug_fp, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;

    default:
        break;
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    static float scalar = 1.0f;
    const float scalar_increment = 0.1f;

    if (yoffset > 0)
        scalar += scalar_increment;
    else if (yoffset < 0 && scalar > 0)
        scalar -= scalar_increment;
    else
        scalar = 0.1f;

    fprintf(debug_fp, "DEBUG VIEW MAGNIFICATION: scalar value = %.2f\n", scalar);

    // mat4x4_scale(v, v, yoffset);
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
    FILE *vs_fp = fopen(vertex_shader_filepath, "r");
    FILE *fs_fp = fopen(fragment_shader_filepath, "r");
    char *vs_text;
    char *fs_text;
    size_t rc;

    GLuint vertex_shader, fragment_shader;


    if (!vs_fp || !fs_fp) {
        glfwTerminate();
        fclose(debug_fp);
        exit(1);
    }

    const size_t vs_size = get_file_size(vs_fp);
    const size_t fs_size = get_file_size(fs_fp);

    vs_text = malloc(sizeof(char)*vs_size);
    fs_text = malloc(sizeof(char)*fs_size);

    if (!vs_text || !fs_text) {
        glfwTerminate();
        fclose(debug_fp);
        exit(1);
    }

    rc = fread(vs_text, sizeof(char), vs_size, vs_fp);
    // fprintf(debug_fp, "VERTEX SHADER READ: read %i bytes\n\n%s\n\n", rc, vs_text);
    // if (rc != vs_size) {
    //     fclose(debug_fp);
    //     exit(1);
    // }
    rc = fread(fs_text, sizeof(char), fs_size, fs_fp);
    // fprintf(debug_fp, "FRAGMENT SHADER READ: read %i bytes\n\n%s\n\n", rc, fs_text);
    // if (rc != fs_size) {
    //     fclose(debug_fp);
    //     exit(1);
    // }

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

    free(vs_text);
    free(fs_text);
}

static void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size)
{
    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, v_size, vertices, GL_STATIC_DRAW);
}

static void vertex_buffer_draw(const GLuint VBO, const float ratio, const float x, const float y, const float z)
{
    mat4x4 m, p, vp, mvp;

    mat4x4_translate(m, x, y, z);
    mat4x4_identity(v);
    mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);
    // mat4x4_mul(mvp, vp, m);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /**
     * These attributes need to be reassociated with the currently bound vertex buffer object for the shader
     * to properly set the variables.
     */
    glEnableVertexAttribArray((GLuint)vpos_location);
    glVertexAttribPointer((GLuint)vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray((GLuint)vcol_location);
    glVertexAttribPointer((GLuint)vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) (sizeof(float) * 2));

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS);
}

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO)
{
    while (!glfwWindowShouldClose(window)) {
        
        float ratio;
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float)width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        vertex_buffer_draw(VBO[0], ratio, 0.0f, -0.5f, 0.0f);
        vertex_buffer_draw(VBO[1], ratio, 1.0f, 0.0f, 0.0f);
        vertex_buffer_draw(VBO[2], ratio, -1.0f, 0.0f, 0.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();

        busy_wait_ms(10);
    }
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

static void busy_wait_ms(const float delay_in_ms)
{
    const float clocks_per_ms = (float)CLOCKS_PER_SEC / 1000;
    const float start_tick = clocks_per_ms * clock();
    const float end_tick = start_tick + (clocks_per_ms * delay_in_ms);

    while (clocks_per_ms * clock() <= end_tick);
}
