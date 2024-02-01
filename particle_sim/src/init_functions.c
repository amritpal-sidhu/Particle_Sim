
#include "particle_sim.h"


static GLFWerrorfun previous_error_callback;


void create_particle_vertices(struct vertex *p_vertices, struct vertex *e_vertices)
{
    /**
     * Populate particle vertex point array for drawing with OpenGL
     */
    #ifdef DRAW_SPHERE
    const vector3d_t sphere_center = {0};
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

    log__write(log_handle, LOG_DATA, "particle_id,mass,charge,x_momenta,y_momenta,z_momenta,x_pos,y_pos,z_pos,pitch_momenta,roll_momenta,yaw_momenta,pitch,roll,yaw");
    #endif
}

void create_particle_objects(particle_t **particles)
{
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
}

void init_opengl_libraries(GLFWwindow **window)
{
    const int initial_window_width = 1280;
    const int initial_window_height = 960;
    int version, major, minor, rev;

    previous_error_callback = glfwSetErrorCallback(error_callback);

    /**
     * initialize glfw
     */
    ERROR_CHECK(!glfwInit(), exit(EXIT_FAILURE), LOG_ERROR, "glfwInit() failed");
 
    /**
     * use the context version and profile hints provided by cmake based on which
     * library glad creates
     */
    #if defined(TARGET_GL_MAJOR) && defined(TARGET_GL_MINOR) && defined (TARGET_GL_PROFILE)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, TARGET_GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, TARGET_GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, TARGET_GL_PROFILE);
    #else
    log__write(log_handle, LOG_WARNING, "OPENGL API MAJOR,MINOR, and PROFILE target macros are not defined");
    #endif
    #ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif

    /**
     * Create window and make it the current context
     */
    *window = glfwCreateWindow(initial_window_width, initial_window_height, "Particle Sim", NULL, NULL);
    ERROR_CHECK(!(*window), exit(EXIT_FAILURE), LOG_ERROR, "glfwCreateWindow() failed");
    /* Use the video mode of the primary monitor to make the initial window position centered */
    const GLFWvidmode *video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    ERROR_CHECK(!video_mode, exit(EXIT_FAILURE), LOG_ERROR, "glfwGetVideoMode() failed");
    glfwSetWindowPos(*window, (video_mode->width-initial_window_width)/2, (video_mode->height-initial_window_height)/2);

    /* Make the context of the window current for the calling thread */    
    glfwMakeContextCurrent(*window);
    
    /* load glad  */
    ERROR_CHECK(!(version=gladLoadGL(glfwGetProcAddress)), exit(EXIT_FAILURE), LOG_ERROR, "gladLoadGL() failed");

    /* log library information */
    glfwGetVersion(&major, &minor, &rev);
    log__write(log_handle, LOG_NONE, "      GLFW Version Data:");
    log__write(log_handle, LOG_INFO, "Compile-time GLFW Version String: %s", glfwGetVersionString());
    log__write(log_handle, LOG_INFO, "Runtime GLFW Version: %i.%i.%i\n", major, minor, rev);
    log__write(log_handle, LOG_NONE, "      OpenGL Version Data:");
    log__write(log_handle, LOG_INFO, "Loaded OpenGL Version: %u.%u", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    log__write(log_handle, LOG_INFO, "GL_VERSION: %s", glGetString(GL_VERSION));
    log__write(log_handle, LOG_INFO, "GL_VENDOR: %s", glGetString(GL_VENDOR));
    log__write(log_handle, LOG_INFO, "GL_RENDERER: %s\n", glGetString(GL_RENDERER));

    /**
     * enable debug context when debug build
     */
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

    #ifdef GLAD_OPTION_GL_DEBUG
    gladSetGLPreCallback(pre_call_gl_callback);
    gladSetGLPostCallback(post_call_gl_callback);
    gladInstallGLDebug();
    #endif
    #endif
    
    /**
     * set callbacks and swap interval
     */
    glfwSetKeyCallback(*window, key_callback);
    glfwSetScrollCallback(*window, scroll_callback);
    glfwSwapInterval(1);
}

void clean_program(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    glfwSetErrorCallback(previous_error_callback);

    glDeleteProgram(rdata.program);
    glDeleteBuffers(BO_COUNT, rdata.VAO);
    glDeleteBuffers(BO_COUNT, rdata.VBO);

    for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
        particle__delete(particles[i]);

    log__write(log_handle, LOG_STATUS, "Program terminated correctly.");
    log__close(log_handle);
    log__delete(log_handle);
}
