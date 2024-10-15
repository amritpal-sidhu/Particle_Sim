#define GLAD_GL_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

#include "particle_sim.h"


static void render_loop(GLFWwindow *window);


/* Global variables */
log_t *log_handle;
particle_t *particles[P_COUNT+E_COUNT];
/* View scalar initial value determined from experimentation, but not sure it's source */
struct render_data_s rdata = {.num_segments = NUM_SEGMENTS, .view_scalar = 10E-20f};


int main(void)
{   
    struct vertex p_vertices[NUM_SEGMENTS];
    struct vertex e_vertices[NUM_SEGMENTS];

    GLFWwindow *window;


    /**
     * open log file
     */
    if (!(log_handle=log__open(DEBUG_OUTPUT_FILEPATH, "w"))) {
        fprintf(stderr, "%s:%u: log__open() failed\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    /**
     * initialize glfw and glad
     */
    init_opengl_libraries(&window);

    /**
     * initialize particle buffers
     */
    create_particle_vertices(p_vertices, e_vertices);
    create_particle_objects(particles);

    /**
     * compile and link shaders and create buffer objects
     */
    ERROR_CHECK(shader_compile_and_link(&rdata), exit(EXIT_FAILURE), LOG_ERROR, "shader_compile_and_link() failed");

    /**
     * initialize buffer objects
     */
    vertex_buffer_init(&rdata, P_BUF, p_vertices, NUM_SEGMENTS);
    vertex_buffer_init(&rdata, E_BUF, e_vertices, NUM_SEGMENTS);
    shader_storage_buffer_init(&rdata, *particles);
    uniform_buffer_init(&rdata);

    /**
     * create and initialize a vertex array object
     */
    vertex_array_object_init(&rdata);

 

    render_loop(window);
    clean_program(window);

    return 0;
}


/* Local function definitions */
static void render_loop(GLFWwindow *window)
{
    static const double wait_time_usec = 10000.0;
    static double loop_start_time;

    glfwSetTime(0);

    while (!glfwWindowShouldClose(window)) {
        
        int width, height;

        loop_start_time = glfwGetTime();
 
        glfwGetFramebufferSize(window, &width, &height);
        rdata.ratio = (float)width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i) {
            render_particles(&rdata, i);
            run_time_evolution_shader(&rdata, i);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        double elapsed_time_usec = 1E6*(glfwGetTime()-loop_start_time);
        if (elapsed_time_usec < wait_time_usec)
            usleep(wait_time_usec-elapsed_time_usec);
    }
}
