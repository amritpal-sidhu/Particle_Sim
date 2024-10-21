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
static void get_MVP_ssbo_data(GLfloat MVP[NUM_PARTICLES][4][4]);


/* Global variables */
log_t *log_handle;
particle_t particles[NUM_PARTICLES];
/* View scalar initial value determined from experimentation, but not sure it's source */
struct render_data_s rdata = {.num_segments = NUM_SEGMENTS, .view_scalar = 1E-20f};


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


        for (size_t i = 0; i < NUM_PARTICLES; ++i) {
            log__write(log_handle, LOG_DEBUG, "particle[%u].pos = <%E, %E, %E>\n", i, particles[i].pos.i, particles[i].pos.j, particles[i].pos.k);
        }

        run_time_evolution_shader(&rdata, particles);
        
        // get_MVP_ssbo_data(MVP);
        // for (size_t i = 0; i < NUM_PARTICLES; ++i) {
        //     log__write(log_handle, LOG_DEBUG, "MVP[%u]: {%E, %E, %E, %E}", i,   MVP[i][0][0], MVP[i][1][0], MVP[i][2][0], MVP[i][3][0]);
        //     log__write(log_handle, LOG_DEBUG, "        {%E, %E, %E, %E}",       MVP[i][0][1], MVP[i][1][1], MVP[i][2][1], MVP[i][3][1]);
        //     log__write(log_handle, LOG_DEBUG, "        {%E, %E, %E, %E}",       MVP[i][0][2], MVP[i][1][2], MVP[i][2][2], MVP[i][3][2]);
        //     log__write(log_handle, LOG_DEBUG, "        {%E, %E, %E, %E}\n",     MVP[i][0][3], MVP[i][1][3], MVP[i][2][3], MVP[i][3][3]);
        // } log__write(log_handle, LOG_NONE, "");

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
    static const double delay_usec = 10000.0;
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

static void get_MVP_ssbo_data(GLfloat MVP[NUM_PARTICLES][4][4])
{
    GLfloat ***mapped_data;

    mapped_data = glMapNamedBuffer(rdata.SSBO[MVP_SSBO], GL_READ_ONLY);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    memcpy(MVP, mapped_data, NUM_PARTICLES*ssbo_info[MVP_SSBO].size);
    glUnmapNamedBuffer(rdata.SSBO[MVP_SSBO]);
}
