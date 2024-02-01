
#include "particle_sim.h"


#ifdef DEBUG
void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
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

void pre_call_gl_callback(const char *name, GLADapiproc apiproc, int len_args, ...)
{
    
}

void post_call_gl_callback(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...)
{

}
#endif

void error_callback(int error, const char *description)
{
    log__write(log_handle, LOG_ERROR, "Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    const double magnify_scalar = 1.25;
    const double minify_scalar = 0.75;
    /** 
     * I'm not sure why these values get used... 
     * might be the size of things in my coordinate system
     */
    const double upper_bound = 10E-20;
    const double lower_bound = 1E-20;

    rdata.view_scalar *= yoffset > 0 ? magnify_scalar : minify_scalar;

    if (rdata.view_scalar > upper_bound)
        rdata.view_scalar = upper_bound;
    else if (rdata.view_scalar < lower_bound)
        rdata.view_scalar = lower_bound;
}
