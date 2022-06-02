#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphic_helpers.h"

#include "vector.h"

#include "particle.h"
#include "mechanics.h"
#include "logging.h"


#define CIRCLE_SEGMENTS     32

#define P_COUNT             1   // Temporary solution to "simulate" a nucleus
#define E_COUNT             2


static void pre_exit_calls(void);

static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO);
static void busy_wait_ms(const float delay_in_ms);


/**
 * Current solution to acos output range of [0, PI]
 */
static void correct_signs(vector3d_t *F, const vector3d_t a, const vector3d_t b, const int attract);


/* Global variables */
log_t *log_handle;

static struct shader_variables shader_vars;

static particle_t *particles[P_COUNT+E_COUNT];

/* Main parameters that will effect the behavior */
static const double sample_period = 8E-3;
static const vector3d_t initial_pos[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = -0.1, .k = 0},
    /* Negatively charged */
    {.i = -0.25, .j = 0, .k = 0},
    {.i = 0.55, .j = 0.25, .k = 0},
    // {.i = 0.5, .j = -0.5, .k = 0},
    // {.i = -0.5, .j = -0.5, .k = 0},
};
static const vector3d_t initial_momentum[P_COUNT+E_COUNT] = {
    /* Positively charged */
    {.i = 0, .j = 0, .k = 0},
    /* Negatively charged */
    {.i = 0, .j = 0, .k = 0},
    {.i = 0, .j = 0, .k = 0},
    // {.i = 0, .j = 0, .k = 0},
    // {.i = 0, .j = 0, .k = 0},
};


/* View scalar initial value determined from experimentation, but not sure it's source */
static struct draw_variables draw_vars = {.num_segments = CIRCLE_SEGMENTS, .view_scalar = 10E-20};


/* Entry point */
int main(void)
{
    const int initial_window_width = 1280;
    const int initial_window_height = 960;
    
    const vector2d_t circle_center = {0};
    struct vertex p_vertices[CIRCLE_SEGMENTS];
    struct vertex e_vertices[CIRCLE_SEGMENTS];
    
    GLuint VBO[P_COUNT+E_COUNT], program;


    if (!(log_handle=logging__open(DEBUG_OUTPUT_FILEPATH, "w")))
        return 1;

    logging__write(log_handle, STATUS, "Log file opened.");

    create_circle_vertex_array(p_vertices, circle_center, FAKE_NUCLEUS_RADI, CIRCLE_SEGMENTS, p_color);
    create_circle_vertex_array(e_vertices, circle_center, FAKE_NUCLEUS_RADI/8, CIRCLE_SEGMENTS, e_color);

    /**
     * Initialization of specific initial coordinates
     * 
     * Currently initializing the "nucleus" as a stable (equal neutrons to protons to electrons)
     * 
     * In reality the nucleus is likely in motion along with spin, which would generate magnetic fields,
     * further complicating this simulation.  Something to work on in the future.
     */
    for (size_t i = 0; i < P_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], initial_momentum[i], E_COUNT*(PROTON_MASS+NEUTRON_MASS), E_COUNT*PROTON_CHARGE);
    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], initial_momentum[i], ELECTRON_MASS, ELECTRON_CHARGE);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        pre_exit_calls();
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(initial_window_width, initial_window_height, "Particle Sim", NULL, NULL);
    if (!window) {
        pre_exit_calls();
        return 1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    
    if (shader_compile_and_link(&program)) {
        pre_exit_calls();
        exit(1);
    }

    shader_vars.mvp_location = glGetUniformLocation(program, "MVP");
    shader_vars.vpos_location = glGetAttribLocation(program, "vPos");
    shader_vars.vcol_location = glGetAttribLocation(program, "vCol");

    for (size_t i = 0; i < P_COUNT; ++i)
        vertex_buffer_init(&VBO[i], p_vertices, sizeof(p_vertices));

    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        vertex_buffer_init(&VBO[i], e_vertices, sizeof(e_vertices));
 
    render_loop(window, program, VBO);

    logging__write(log_handle, STATUS, "Program terminated correctly.\n");

    glfwDestroyWindow(window);
    pre_exit_calls();

    return 0;
}


/* Local function definitions */
static void pre_exit_calls(void)
{
    glfwTerminate();
    logging__close(log_handle);
    logging__delete(log_handle);

    for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
        particle__delete(particles[i]);
}

static void error_callback(int error, const char *description)
{
    logging__write(log_handle, ERROR, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key) {

    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;

    // reset particle locations... but not MOMENTUM!
    case GLFW_KEY_R:
        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
            particles[i]->pos = initial_pos[i];
        break;

    default:
        break;
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    const double magnify_scalar = 1.25;
    const double minify_scalar = 0.75;
    /** 
     * I'm not sure why these values get used... 
     * might be the size of things in my coordinate system
     */
    const double upper_bound = 10E-20;
    const double lower_bound = 1E-20;

    draw_vars.view_scalar *= yoffset > 0 ? magnify_scalar : minify_scalar;

    if (draw_vars.view_scalar > upper_bound)
        draw_vars.view_scalar = upper_bound;
    else if (draw_vars.view_scalar < lower_bound)
        draw_vars.view_scalar = lower_bound;

    // fprintf(debug_fp, "DEBUG VIEW MAGNIFICATION: view_scalar value = %E\n", view_scalar);
}

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO)
{
    while (!glfwWindowShouldClose(window)) {
        
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);
        draw_vars.ratio = (float)width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
            vertex_buffer_draw(VBO[i], shader_vars, draw_vars, particles[i]->pos);

        glfwSwapBuffers(window);
        glfwPollEvents();

        update_positions(particles, P_COUNT+E_COUNT, sample_period);

        busy_wait_ms(10);
    }
}

static void busy_wait_ms(const float delay_in_ms)
{
    const float clocks_per_ms = (float)CLOCKS_PER_SEC / 1000;
    const float start_tick = clocks_per_ms * clock();
    const float end_tick = start_tick + (clocks_per_ms * delay_in_ms);

    while (clocks_per_ms * clock() <= end_tick);
}
