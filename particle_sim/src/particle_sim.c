#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

#include "particle_sim.h"
#include "graphic_helpers.h"
#include "particle.h"
#include "mechanics.h"
#include "log.h"
#include "common.h"


static void clean_program(GLFWwindow *window, GLuint program, GLuint *VBO, GLuint *VAO, size_t count);

#ifdef DEBUG
static void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif
static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO, GLuint *VAO);


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
    
    GLuint VBO[P_COUNT+E_COUNT], VAO[P_COUNT+E_COUNT];
    GLuint program;


    if (!(log_handle=log__open(DEBUG_OUTPUT_FILEPATH, "w"))) {
        fprintf(stderr, "%s:%u: log__open() failed\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

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

    #ifdef LOG_DATA
    for (int i = 0; i < NUM_SEGMENTS; ++i)
        log__write(log_handle, LOG_DATA, "p_vertex[%i] = <%.3f,%.3f,%.3f>", i, p_vertices[i].pos.i, p_vertices[i].pos.j, p_vertices[i].pos.k);
    #endif

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
    ERROR_CHECK(!glfwInit(), exit(EXIT_FAILURE), log_handle, LOG_ERROR, "glfwInit() failed");
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif
    GLFWwindow *window = glfwCreateWindow(initial_window_width, initial_window_height, "Particle Sim", NULL, NULL);
    ERROR_CHECK(!window, exit(EXIT_FAILURE), log_handle, LOG_ERROR, "glfwCreateWindow() failed");

    const GLFWvidmode *video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    ERROR_CHECK(!video_mode, exit(EXIT_FAILURE), log_handle, LOG_ERROR, "glfwGetVideoMode() failed");
    glfwSetWindowPos(window, (video_mode->width-initial_window_width)/2, (video_mode->height-initial_window_height)/2);
    glfwMakeContextCurrent(window);
    
    int version = gladLoadGL(glfwGetProcAddress);
    ERROR_CHECK(!version, exit(EXIT_FAILURE), log_handle, LOG_ERROR, "gladLoadGL() failed");
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    log__write(log_handle, LOG_INFO, "Runtime GLFW Version: %i.%i.%i", major, minor, rev);
    log__write(log_handle, LOG_INFO, "Compile GLFW Version String: %s", glfwGetVersionString());
    log__write(log_handle, LOG_INFO, "Loaded OpenGL Version: %u.%u", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    log__write(log_handle, LOG_INFO, "GL_VERSION: %s", glGetString(GL_VERSION));
    log__write(log_handle, LOG_INFO, "GL_VENDOR: %s", glGetString(GL_VENDOR));
    log__write(log_handle, LOG_INFO, "GL_RENDERER: %s", glGetString(GL_RENDERER));
    

    #ifdef DEBUG
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debug_message_callback, NULL);
        log__write(log_handle, LOG_INFO, "Registered DebugMessageCallback");
    } else
        log__write(log_handle, LOG_ERROR, "Failed to create debug context");
    #endif

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);


    ERROR_CHECK(shader_compile_and_link(&program), exit(EXIT_FAILURE), log_handle, LOG_ERROR, "shader_compile_and_link() failed");

    for (size_t i = 0; i < P_COUNT; ++i)
        vertex_buffer_init(&VBO[i], p_vertices, sizeof(p_vertices));

    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        vertex_buffer_init(&VBO[i], e_vertices, sizeof(e_vertices));

    vertex_array_object_init(VAO, VBO, P_COUNT+E_COUNT);


    #ifdef LOG_DATA
    log__write(log_handle, LOG_DATA, "particle_id,mass,charge,x_momenta,y_momenta,z_momenta,x_pos,y_pos,z_pos,pitch_momenta,roll_momenta,yaw_momenta,pitch,roll,yaw");
    #endif
 
    render_loop(window, program, VBO, VAO);

    clean_program(window, program, VBO, VAO, P_COUNT+E_COUNT);

    return 0;
}


/* Local function definitions */
static void clean_program(GLFWwindow *window, GLuint program, GLuint *VBO, GLuint *VAO, size_t count)
{
    glfwDestroyWindow(window);
    glfwTerminate();

    glDeleteProgram(program);
    glDeleteBuffers(count, VBO);
    glDeleteBuffers(count, VAO);

    for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
        particle__delete(particles[i]);

    log__write(log_handle, LOG_STATUS, "Program terminated correctly.");
    log__close(log_handle);
    log__delete(log_handle);
}

#ifdef DEBUG
static void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    char source_string[48], type_string[48], severity_string[48];

    switch (source) {
        case GL_DEBUG_SOURCE_API: strncpy(source_string, "GL_DEBUG_SOURCE_API", sizeof(source_string)); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: strncpy(source_string, "GL_DEBUG_SOURCE_WINDOW_SYSTEM", sizeof(source_string)); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: strncpy(source_string, "GL_DEBUG_SOURCE_SHADER_COMPILER", sizeof(source_string)); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: strncpy(source_string, "GL_DEBUG_SOURCE_THIRD_PARTY", sizeof(source_string)); break;
        case GL_DEBUG_SOURCE_APPLICATION: strncpy(source_string, "GL_DEBUG_SOURCE_APPLICATION", sizeof(source_string)); break;
        case GL_DEBUG_SOURCE_OTHER: strncpy(source_string, "GL_DEBUG_SOURCE_OTHER", sizeof(source_string)); break;
        case GL_DONT_CARE: strncpy(source_string, "GL_DONT_CARE", sizeof(source_string)); break;
        default: strncpy(source_string, "OTHER_SOURCE", sizeof(source_string)); break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: strncpy(type_string, "GL_DEBUG_TYPE_ERROR", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strncpy(type_string, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: strncpy(type_string, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_PORTABILITY: strncpy(type_string, "GL_DEBUG_TYPE_PORTABILITY", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_PERFORMANCE: strncpy(type_string, "GL_DEBUG_TYPE_PERFORMANCE", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_MARKER: strncpy(type_string, "GL_DEBUG_TYPE_MARKER", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_PUSH_GROUP: strncpy(type_string, "GL_DEBUG_TYPE_PUSH_GROUP", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_POP_GROUP: strncpy(type_string, "GL_DEBUG_TYPE_POP_GROUP", sizeof(type_string)); break;
        case GL_DEBUG_TYPE_OTHER: strncpy(type_string, "GL_DEBUG_TYPE_OTHER", sizeof(type_string)); break;
        case GL_DONT_CARE: strncpy(type_string, "GL_DONT_CARE", sizeof(type_string)); break;
        default: strncpy(type_string, "OTHER_TYPE", sizeof(type_string)); break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW: strncpy(severity_string, "GL_DEBUG_SEVERITY_LOW", sizeof(severity_string)); break;
        case GL_DEBUG_SEVERITY_MEDIUM: strncpy(severity_string, "GL_DEBUG_SEVERITY_MEDIUM", sizeof(severity_string)); break;
        case GL_DEBUG_SEVERITY_HIGH: strncpy(severity_string, "GL_DEBUG_SEVERITY_HIGH", sizeof(severity_string)); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: strncpy(severity_string, "GL_DEBUG_SEVERITY_NOTIFICATION", sizeof(severity_string)); break;
        case GL_DONT_CARE: strncpy(severity_string, "GL_DONT_CARE", sizeof(severity_string)); break;
        default: strncpy(severity_string, "OTHER_SEVERITY", sizeof(severity_string)); break;
    }

    log__write(log_handle, LOG_DEBUG, "Source: %s, Type: %s, Severity: %s, ID: %u, Message: %s", source_string, type_string, severity_string, id, message);
}
#endif

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
}

static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO, GLuint *VAO)
{
    static const double wait_time_usec = 10000.0;
    static double loop_start_time;

    glfwSetTime(0);

    while (!glfwWindowShouldClose(window)) {
        
        int width, height;

        loop_start_time = glfwGetTime();
 
        glfwGetFramebufferSize(window, &width, &height);
        draw_vars.ratio = (float)width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i) {
            draw_vars.pos = particles[i]->pos;
            draw_vars.angle = particles[i]->orientation;
            glBindVertexArray(VAO[i]);
            vertex_buffer_draw(program, draw_vars);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        time_evolution(particles, P_COUNT+E_COUNT, sample_period);

        double elapsed_time_usec = 1E6*(glfwGetTime()-loop_start_time);
        if (elapsed_time_usec < wait_time_usec)
            usleep(wait_time_usec-elapsed_time_usec);
    }
}
