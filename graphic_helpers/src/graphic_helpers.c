#include "graphic_helpers.h"

#include <stdlib.h>

#include "particle_sim.h"
#include "particle.h"
#include "common.h"


#define SHADER_COUNT    3
#define COMPUTE_SHADER_FILEPATH     "shaders/time_evolution.comp.glsl"
#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert.glsl"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag.glsl"

enum shader_e {
    COMP,
    VERT,
    FRAG,
};

enum shader_build_e {
    COMPILE,
    LINK,
};

/* Global variables */
extern log_t *log_handle;

static const GLchar *shader_filepath[SHADER_COUNT] = {
    COMPUTE_SHADER_FILEPATH,
    VERTEX_SHADER_FILEPATH,
    FRAGMENT_SHADER_FILEPATH
};

static const  GLenum shader_type[SHADER_COUNT] = {
    GL_COMPUTE_SHADER,
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
};

const color_t p_color = (color_t){.r = 1.0f, .g = 0.0f, .b = 0.0f};
const color_t e_color = (color_t){.r = 0.0f, .g = 0.0f, .b = 1.0f};


/* Private function definitions */
static GLchar *get_shader_text(const enum shader_e type, size_t *text_len)
{
    GLchar *text = NULL;
    FILE *fp = fopen(shader_filepath[type], "r");
    
    if (text_len) *text_len = 0;
    if (!fp) return NULL;

    const size_t file_size = get_file_size(fp);
    if (text_len) *text_len = file_size;

    if (!(text=malloc(sizeof(char)*file_size+1))) {
        fclose(fp);
        return NULL;
    }

    char shader_name[16];
    switch (type) {
        case COMP: strncpy(shader_name, "COMPUTE", sizeof(shader_name)); break;
        case VERT: strncpy(shader_name, "VERTEX", sizeof(shader_name)); break;
        case FRAG: strncpy(shader_name, "FRAGMENT", sizeof(shader_name)); break;
        default: strncpy(shader_name, "WRONG_TYPE", sizeof(shader_name)); break;
    }

    if (file_size != fread(text, sizeof(char), file_size, fp)) {
        log__write(log_handle, LOG_WARNING, "%s SHADER READ: read incorrect bytes\n\n%s\n", shader_name, text);
        free(text);
        fclose(fp);
        return NULL;
    }
    text[sizeof(char)*file_size] = '\0';

    fclose(fp);
    return text;
}

/**
 * @retval On error 0 is returned
 *         On success the value of the shader/program object reference is returned
 */
static GLuint check_shader_build_status(const enum shader_build_e stage, const GLuint obj_ref)
{
    int status;
    
    switch (stage) {
        case COMPILE: glGetShaderiv(obj_ref, GL_COMPILE_STATUS, &status); break;
        case LINK: glGetProgramiv(obj_ref, GL_LINK_STATUS, &status); break;
        default: return 0; break;
    }

    if (!status) {
        char info[512];
        switch (stage) {
            case COMPILE: glGetShaderInfoLog(obj_ref, sizeof(info), NULL, info); break;
            case LINK: glGetProgramInfoLog(obj_ref, sizeof(info), NULL, info); break;
            default: return 0; break;
        }
        log__write(log_handle, LOG_ERROR, "%s", info);
        return 0;
    }

    return obj_ref;
}

static GLuint compile_shader(const enum shader_e type)
{
    GLchar *text;

    if (!(text=get_shader_text(type, NULL)))
        return 0;

    GLuint shader = glCreateShader(shader_type[type]);
    glShaderSource(shader, 1, (const GLchar * const*)&text, NULL);
    free(text);
    glCompileShader(shader);
    return check_shader_build_status(COMPILE, shader);
}

static GLuint compile_shader_spir_v(const enum shader_e type)
{
    GLchar *text;
    size_t text_length;

    if (!(text=get_shader_text(type, &text_length)))
        return 0;

    GLuint shader = glCreateShader(shader_type[type]);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, text, text_length);
    free(text);
    return check_shader_build_status(COMPILE, shader);
}

static GLuint link_shader(const GLuint *shaders, const size_t s_size)
{
    GLuint program = glCreateProgram();
    for (size_t i = 0; i < s_size; ++i)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);
    return check_shader_build_status(LINK, program);
}

/* Public function definitions */
int shader_compile_and_link(struct render_data_s *rdata)
{
    GLuint shaders[SHADER_COUNT];
    int retval = EXIT_SUCCESS;

    /* get the shader source and compile */
    for (enum shader_e s = COMP; s <= FRAG; ++s) {
        if (!(shaders[s]=compile_shader(s)))
            return EXIT_FAILURE;
    }

    /* link programs */
    if (!(rdata->compute_program=link_shader(&shaders[COMP], 1)) || !(rdata->program=link_shader(&shaders[VERT], SHADER_COUNT-1)))
        retval = EXIT_FAILURE;
    
    /* clean up shader objects */
    glDetachShader(rdata->compute_program, shaders[0]);
    glDeleteShader(shaders[0]);
    for (enum shader_e s = VERT; s <= FRAG; ++s) {
        glDetachShader(rdata->program, shaders[s]);
        glDeleteShader(shaders[s]);
    }

    return retval;
}

int shader_compile_and_link_spir_v(struct render_data_s *rdata)
{
    GLuint shaders[SHADER_COUNT];

    for (enum shader_e s = VERT; s <= FRAG; ++s) {
        if (!(shaders[s]=compile_shader_spir_v(s)))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void bind_vertex_attributes(const struct render_data_s *rdata)
{
    for (buffer_index_e buf = P_BUF; buf <= E_BUF; ++buf) {

        glBindVertexArray(rdata->VAO[buf]);
        glBindVertexBuffer(buf, rdata->VBO[buf], 0, sizeof(struct vertex));
        glEnableVertexAttribArray(IN_POS_ATTR_LOC);
        glVertexAttribFormat(IN_POS_ATTR_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, pos));
        glVertexAttribBinding(IN_POS_ATTR_LOC, buf);
        glEnableVertexAttribArray(IN_COL_ATTR_LOC);
        glVertexAttribFormat(IN_COL_ATTR_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, color));
        glVertexAttribBinding(IN_COL_ATTR_LOC, buf);
    }

    glBindVertexArray(0);
}

void vertex_array_object_init(struct render_data_s *rdata)
{
    glGenVertexArrays(BO_COUNT, rdata->VAO);
    bind_vertex_attributes(rdata);
}

void vertex_buffer_init(GLuint *VBO, void *data, const size_t size)
{
    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shader_storage_buffer_init(struct render_data_s *rdata, void *data, const size_t size)
{
    glGenBuffers(1, &rdata->SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_BINDING_POINT, rdata->SSBO);
}

void render_particles(const struct render_data_s *rdata, const size_t particle_index)
{
    glUseProgram(rdata->program);
    glUniform1ui(VS_ID_UNIFORM_LOC, particle_index);
    glUniform1f(VIEW_SCALAR_UNIFORM_LOC, rdata->view_scalar);
    glUniform1f(VIEW_SCALAR_UNIFORM_LOC, rdata->ratio);
    glBindVertexArray(rdata->VAO[particle_index<P_COUNT?P_BUF:E_BUF]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, rdata->num_segments);
    glBindVertexArray(0);

}

void run_time_evolution_shader(const struct render_data_s *rdata, const size_t particle_index, const float sample_period)
{
    glUseProgram(rdata->compute_program);
    glUniform1ui(CS_ID_UNIFORM_LOC, particle_index);
    glUniform1f(SAMPLE_PERIOD_UNIFORM_LOC, sample_period);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color)
{
    for(int a = 0; a < num_segments; ++a) {
        const double theta = 2.0f * PI * a / num_segments;

        const double x = r * cos(theta);
        const double y = r * sin(theta);

        v[a].pos.i = x + center.i;
        v[a].pos.j = y + center.j;
        v[a].color = color;
    }
}

void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color)
{
    v[0].pos.i = 0 + center.i;
    v[0].pos.j = 0 + center.j;
    v[0].pos.k = r + center.k;
    v[0].color = color;

    for (int a = 0; a < num_z_segments; ++a) {

        const double phi = PI * (a+1) / num_z_segments;

        for(int b = 0; b < num_y_segments; ++b) {

            const int index = (num_z_segments * a) + b + 1;

            const double theta = 2.0 * PI * b / num_y_segments;

            const double x = r * cos(theta) * sin(phi);
            const double y = r * sin(theta) * sin(phi);
            const double z = r * cos(phi);
            
            v[index].pos.i = x + center.i;
            v[index].pos.j = y + center.j;
            v[index].pos.k = z + center.k;
            if (!(a % 2))
                v[index].color = (color_t){1,1,1};
            else
                v[index].color = color;
        }
    }

    v[num_z_segments*num_y_segments+1].pos.i = 0 + center.i;
    v[num_z_segments*num_y_segments+1].pos.j = 0 + center.j;
    v[num_z_segments*num_y_segments+1].pos.k = -r + center.k;
    v[num_z_segments*num_y_segments+1].color = color;

}
