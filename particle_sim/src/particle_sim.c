#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "vector.h"

#include "particle.h"
#include "mechanic_equations.h"


#define PI                  3.14159265358979323846264338327950
#define CIRCLE_SEGMENTS     32

#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag"
#define DEBUG_OUTPUT_FILEPATH       "debug_output.txt"

/* Playing with the numbers for now ("simulating" Helium in the making... kind of) */
#define P_COUNT             1
#define E_COUNT             2


struct vertex
{
    vector3d_t pos;
    color_t color;
};


static void pre_exit_calls(void);

static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static size_t get_file_size(FILE *fp);
static void shader_compile_and_link(GLuint *program);

static void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size);
static void vertex_buffer_draw(const GLuint VBO, const float ratio, const vector3d_t pos);
static void render_loop(GLFWwindow *window, const GLuint program, GLuint *VBO);

static void update_positions(void);
static void correct_signs(vector3d_t *F, const vector3d_t a, const vector3d_t b, const int sign);

static void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color);
static void busy_wait_ms(const float delay_in_ms);


/**
 * Local global variables
 *   TODO: Use parameter passing once the code becomes
 *         more stable.
 */
static GLint vpos_location, vcol_location, mvp_location;

static particle_t *particles[P_COUNT+E_COUNT];
static const vector3d_t initial_pos[P_COUNT+E_COUNT] = {
    {.i = 0, .j = -0.1, .k = 0},
    {.i = -0.75, .j = 0.25, .k = 0},
    {.i = 0.75, .j = 0, .k = 0},
};

static double view_scalar = 10E-20; // Determined from experimentation, but not sure it's source

static FILE *debug_fp;


/* Entry point */
int main(void)
{
    const int initial_window_width = 1280;
    const int initial_window_height = 960;
    
    const vector2d_t circle_center = {0};
    const double p_radius = 0.1;
    const double e_radius = 0.05;
    const color_t p_color = {1.0f, 0.0f, 0.0f};
    const color_t e_color = {0.0f, 0.0f, 1.0f};
    struct vertex p_vertices[CIRCLE_SEGMENTS];
    struct vertex e_vertices[CIRCLE_SEGMENTS];
    
    GLuint VBO[P_COUNT+E_COUNT], program;


    printf("Attempting to open debug file\n");
    debug_fp = fopen(DEBUG_OUTPUT_FILEPATH, "w");
    if (!debug_fp) {
        printf("failed to open debug file\n");
        return 1;
    }

    fprintf(debug_fp, "*** DEBUG OUTPUT LOG ***\n\n");

    create_circle_vertex_array(p_vertices, circle_center, p_radius, CIRCLE_SEGMENTS, p_color);
    create_circle_vertex_array(e_vertices, circle_center, e_radius, CIRCLE_SEGMENTS, e_color);

    /**
     * Initialization of specific initial coordinates
     */
    for (size_t i = 0; i < P_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], (vector3d_t){0}, PROTON_MASS, PROTON_CHARGE);
    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        particles[i] = particle__new(i, initial_pos[i], (vector3d_t){0}, ELECTRON_MASS, ELECTRON_CHARGE);

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
    
    shader_compile_and_link(&program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    for (size_t i = 0; i < P_COUNT; ++i)
        vertex_buffer_init(&VBO[i], p_vertices, sizeof(p_vertices));

    for (size_t i = P_COUNT; i < P_COUNT+E_COUNT; ++i)
        vertex_buffer_init(&VBO[i], e_vertices, sizeof(e_vertices));
 
    render_loop(window, program, VBO);

    fprintf(debug_fp, "Program terminated correctly.\n");

    glfwDestroyWindow(window);
    pre_exit_calls();

    return 0;
}


/* Local function definitions */
static void pre_exit_calls(void)
{
    glfwTerminate();
    fclose(debug_fp);

    for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
        particle__delete(particles[i]);
}

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

    view_scalar *= yoffset > 0 ? magnify_scalar : minify_scalar;

    if (view_scalar > upper_bound)
        view_scalar = upper_bound;
    else if (view_scalar < lower_bound)
        view_scalar = lower_bound;

    // fprintf(debug_fp, "DEBUG VIEW MAGNIFICATION: view_scalar value = %E\n", view_scalar);
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
    FILE *vs_fp = fopen(VERTEX_SHADER_FILEPATH, "r");
    FILE *fs_fp = fopen(FRAGMENT_SHADER_FILEPATH, "r");
    char *vs_text;
    char *fs_text;
    size_t rc;

    GLuint vertex_shader, fragment_shader;


    if (!vs_fp || !fs_fp) {
        pre_exit_calls();
        exit(1);
    }

    const size_t vs_size = get_file_size(vs_fp);
    const size_t fs_size = get_file_size(fs_fp);

    vs_text = malloc(sizeof(char)*vs_size);
    fs_text = malloc(sizeof(char)*fs_size);

    if (!vs_text || !fs_text) {
        pre_exit_calls();
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

static void vertex_buffer_draw(const GLuint VBO, const float ratio, const vector3d_t pos)
{
    mat4x4 m, p, mvp;

    mat4x4_translate(m, pos.i, pos.j, pos.k);
    mat4x4_scale(m, m, view_scalar);
    mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /**
     * These attributes need to be reassociated with the currently bound vertex buffer object for the shader
     * to properly set the variables.
     */
    glEnableVertexAttribArray((GLuint)vpos_location);
    glVertexAttribPointer((GLuint)vpos_location, 3, GL_DOUBLE, GL_FALSE, sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray((GLuint)vcol_location);
    glVertexAttribPointer((GLuint)vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) (sizeof(double) * 3));

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
        for (size_t i = 0; i < P_COUNT+E_COUNT; ++i)
            vertex_buffer_draw(VBO[i], ratio, particles[i]->pos);

        glfwSwapBuffers(window);
        glfwPollEvents();

        update_positions();

        busy_wait_ms(10);
    }
}

static void update_positions(void)
{
    const double fake_sample_period = 4E-3;
    static vector3d_t impulse_integral[P_COUNT+E_COUNT];

    /* Leave the positively charge particles static for now */
    for (size_t current_id = P_COUNT; current_id < P_COUNT+E_COUNT; ++current_id) {

        vector3d_t F[P_COUNT+E_COUNT-1];
        vector3d_t F_resultant = {0};

        /* Try to find a time improvement to compute all forces acting on current particle */
        for (size_t other_id = 0; other_id < P_COUNT+E_COUNT; ++other_id) {

            if (particles[current_id]->id == other_id) continue;

            /* Assuming positively charged "object" is a nucleus */
            const double charge_of_other = particles[other_id]->charge > 0 ? P_COUNT*particles[other_id]->charge : particles[other_id]->charge;

            const double F_mag = electric_force(particles[current_id]->charge, charge_of_other, vector3d__distance(particles[other_id]->pos, particles[current_id]->pos));
            const double theta = vector3d__theta(particles[other_id]->pos, particles[current_id]->pos);
            F[other_id] = (vector3d_t){.i = F_mag*cos(theta), .j = F_mag*sin(theta)};
            correct_signs(&F[other_id], particles[current_id]->pos, particles[other_id]->pos, particles[other_id]->charge > 0 ? 1 : -1);
        }

        for (size_t other_id = 0; other_id < P_COUNT+E_COUNT; ++other_id)
            F_resultant = vector3d__add(F_resultant, F[other_id]);

        particles[current_id]->vel = velocity_induced_by_force(&impulse_integral[current_id], F_resultant, particles[current_id]->mass, fake_sample_period);

        particles[current_id]->pos.i += fake_sample_period * particles[current_id]->vel.i;
        particles[current_id]->pos.j += fake_sample_period * particles[current_id]->vel.j;
    }
}

static void correct_signs(vector3d_t *F, const vector3d_t a, const vector3d_t b, const int sign)
{
    const vector3d_t F_dir = sign == 1 ? vector3d__sub(b, a) : vector3d__sub(a,b);

    if ((F->i > 0 && F_dir.i < 0) || (F->i < 0 && F_dir.i > 0)) F->i *= -1;
    if ((F->j > 0 && F_dir.j < 0) || (F->j < 0 && F_dir.j > 0)) F->j *= -1;
    if ((F->k > 0 && F_dir.k < 0) || (F->k < 0 && F_dir.k > 0)) F->k *= -1;
}

static void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color)
{
    for(int i = 0; i < num_segments; ++i) {
        double theta = 2.0f * PI * i / (num_segments - 2);

        double x = r * cos(theta);
        double y = r * sin(theta);

        v[i].pos.i = x + center.i;
        v[i].pos.j = y + center.j;
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
