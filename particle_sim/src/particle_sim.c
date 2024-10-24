#define GLAD_GL_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

#include "particle_sim.h"


typedef enum
{
    START,
    STOP,
} delay_e;

static void render_loop(GLFWwindow *window);
static void delay_usec(const delay_e type);
static void print_particle_data_to_log(void);


/* Global variables */
log_t *log_handle;
particle_t particles[NUM_PARTICLES];
/* View scalar initial value determined from experimentation, but not sure it's source */
struct render_data_s rdata = {.num_segments = NUM_SEGMENTS, .view_scalar = 5E-6f};


int main(void)
{   
    struct vertex p_vertices[NUM_SEGMENTS];
    struct vertex e_vertices[NUM_SEGMENTS];

    GLFWwindow *window;


    /* open log file */
    if (!(log_handle=log__open(DEBUG_OUTPUT_FILEPATH, "w"))) {
        fprintf(stderr, "%s:%u: log__open() failed\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // log_level_t levels[] = {LOG_DEBUG, LOG_NONE};
    // log__disable_log_levels(log_handle, levels, 2);

    /* initialize glfw and glad */
    opengl_libraries_init(&window);

    /* compile and link shaders */
    shader_compile_and_link(&rdata);

    /* generate particle vertices and initial physics data */
    create_particle_vertices(p_vertices, e_vertices);
    create_particle_objects(particles);

    /* initialize buffer objects */
    buffer_objects_init(&rdata, p_vertices, e_vertices, particles);

    /* create and initialize a vertex array object */
    vertex_array_object_init(&rdata);

    /* run main render loop and clean program on program termination */
    render_loop(window);
    clean_program(window);

    return 0;
}


/* Local function definitions */
static void render_loop(GLFWwindow *window)
{
    GLfloat MVP[NUM_PARTICLES][4][4];

    glfwSetTime(0);

    while (!glfwWindowShouldClose(window)) {
        
        delay_usec(START);
 
        glfwGetFramebufferSize(window, &rdata.width, &rdata.height);
        rdata.ratio = (float)rdata.width / rdata.height;

        glViewport(0, 0, rdata.width, rdata.height);
        glClear(GL_COLOR_BUFFER_BIT);


        print_particle_data_to_log();

        run_time_evolution_shader(&rdata, particles);
        
        for (size_t i = 0; i < NUM_PARTICLES; ++i)
            render_particles(&rdata, i, particles);

        glfwSwapBuffers(window);
        glfwPollEvents();

        delay_usec(STOP);
    }
}

static void delay_usec(const delay_e type)
{
    static double epoch;
    static const double delay_usec = sample_period * 1E6; // sample_period is in milliseconds
    double elapsed_usec;

    switch (type) {
    case START:
        epoch = glfwGetTime();
        break;

    case STOP:
        elapsed_usec = 1E6*(glfwGetTime()-epoch);
        if (elapsed_usec < delay_usec)
            usleep(delay_usec-elapsed_usec);
        break;
    
    default:
        log__write(log_handle, LOG_ERROR, "delay_usec() 'type' argument is invalid");
        exit(EXIT_FAILURE);
        break;
    }
}

static void print_particle_data_to_log(void)
{
    static size_t sample_number = 0;

    for (size_t i = 0; i < NUM_PARTICLES; ++i) {
        log__write(log_handle, LOG_DATA, "(%llu) particle[%llu].position = <%.3E, %.3E, %.3E>", sample_number, i, particles[i].pos.i, particles[i].pos.j, particles[i].pos.k);
        log__write(log_handle, LOG_DATA, "(%llu) particle[%llu].momentum = <%.3E, %.3E, %.3E>", sample_number, i, particles[i].momenta.i, particles[i].momenta.j, particles[i].momenta.k);
        log__write(log_handle, LOG_DATA, "(%llu) particle[%llu].orientation = <%.3E, %.3E, %.3E>", sample_number, i, particles[i].orientation.i, particles[i].orientation.j, particles[i].orientation.k);
        log__write(log_handle, LOG_DATA, "(%llu) particle[%llu].angular_momentum = <%.3E, %.3E, %.3E>", sample_number, i, particles[i].angular_momenta.i, particles[i].angular_momenta.j, particles[i].angular_momenta.k);
    }      

    ++sample_number;
}
