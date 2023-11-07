#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "particle_sim.h"
#include "graphic_helpers.h"
#include "particle.h"
#include "mechanics.h"
#include "log.h"


static void pre_exit_calls(void);

static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO);
static void busy_wait_ms(const float delay_in_ms);


/* Global variables */
log_t *log_handle;

static particle_t *particles[P_COUNT+E_COUNT];


/* View scalar initial value determined from experimentation, but not sure it's source */
static struct draw_variables draw_vars = {.num_segments = NUM_SEGMENTS, .view_scalar = 10E-20};


/* Entry point */
int main(void)
{
    const int initial_window_width = 1280;
    const int initial_window_height = 960;
    
    const vector3d_t sphere_center = {0};
    struct vertex p_vertices[NUM_SEGMENTS];
    struct vertex e_vertices[NUM_SEGMENTS];
    
    GLuint VBO[P_COUNT+E_COUNT], program;


    if (!(log_handle=log__open(DEBUG_OUTPUT_FILEPATH, "w")))
        return 1;

    log__write(log_handle, LOG_STATUS, "Log file opened.");

    /**
     * Populate particle vertex point array for drawing with OpenGL
     */
    #ifdef __DRAW_SPHERE
    create_sphere_vertex_array(p_vertices, sphere_center, FAKE_NUCLEUS_RADIUS, CIRCLE_Y_SEGMENTS, CIRCLE_Z_SEGMENTS, p_color);
    create_sphere_vertex_array(e_vertices, sphere_center, FAKE_NUCLEUS_RADIUS/8, CIRCLE_Y_SEGMENTS, CIRCLE_Z_SEGMENTS, e_color);
    #else
    const vector2d_t circle_center = {0};
    create_circle_vertex_array(p_vertices, circle_center, FAKE_NUCLEUS_RADIUS, CIRCLE_Y_SEGMENTS, p_color);
    create_circle_vertex_array(e_vertices, circle_center, FAKE_NUCLEUS_RADIUS/8, CIRCLE_Y_SEGMENTS, e_color);
    #endif

    for (int i = 0; i < NUM_SEGMENTS; ++i)
        log__write(log_handle, LOG_INFO, "p_vertex[%i] = <%.3f,%.3f,%.3f>", i, p_vertices[i].pos.i, p_vertices[i].pos.j, p_vertices[i].pos.k);

    /**
     * Creation of particle struct
     * 
     * Currently initializing the "nucleus" as a stable (equal neutrons to protons to electrons)
     * 
     * In reality the nucleus is likely in motion along with spin, which would generate magnetic fields,
     * further complicating this simulation.  Something to work on in the future.
     */
    for (size_t i = 0; i < P_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], initial_momentum[i], initial_orientation[i], initial_angular_momentum[i], E_COUNT*(PROTON_MASS+NEUTRON_MASS), E_COUNT*PROTON_CHARGE, FAKE_NUCLEUS_RADIUS);
    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], initial_momentum[i], initial_orientation[i], initial_angular_momentum[i], ELECTRON_MASS, ELECTRON_CHARGE, FAKE_NUCLEUS_RADIUS/8);

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

    draw_vars.shader_vars.mvp_location = glGetUniformLocation(program, "MVP");
    draw_vars.shader_vars.vpos_location = glGetAttribLocation(program, "vPos");
    draw_vars.shader_vars.vcol_location = glGetAttribLocation(program, "vCol");

    for (size_t i = 0; i < P_COUNT; ++i)
        vertex_buffer_init(&VBO[i], p_vertices, sizeof(p_vertices));

    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        vertex_buffer_init(&VBO[i], e_vertices, sizeof(e_vertices));


    log__write(log_handle, LOG_DATA, "particle_id,mass,charge,x_momenta,y_momenta,z_momenta,x_pos,y_pos,z_pos,pitch_momenta,roll_momenta,yaw_momenta,pitch,roll,yaw");
    render_loop(window, program, VBO);

    log__write(log_handle, LOG_STATUS, "Program terminated correctly.");

    glfwDestroyWindow(window);
    pre_exit_calls();

    return 0;
}


/* Local function definitions */
static void pre_exit_calls(void)
{
    glfwTerminate();
    log__close(log_handle);
    log__delete(log_handle);

    for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
        particle__delete(particles[i]);
}

static void error_callback(int error, const char *description)
{
    log__write(log_handle, LOG_ERROR, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key) {

    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;

    // reset particle locations... but not momenta!
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
        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i) {
            draw_vars.pos = particles[i]->pos;
            draw_vars.angle = particles[i]->orientation;
            vertex_buffer_draw(VBO[i], draw_vars);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        time_evolution(particles, P_COUNT+E_COUNT, sample_period);

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
