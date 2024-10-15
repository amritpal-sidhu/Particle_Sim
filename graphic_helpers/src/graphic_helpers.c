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

void vertex_buffer_init(struct render_data_s *rdata, const buffer_index_e buf, void *data, const size_t size)
{
    glGenBuffers(1, &rdata->VBO[buf]);
    glBindBuffer(GL_ARRAY_BUFFER, rdata->VBO[buf]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shader_storage_buffer_init(struct render_data_s *rdata, void *particle_data)
{
    GLint num_resources, ssbo_max_len;
    GLsizei string_len;
    GLchar resource_name[32];

    glGenBuffers(1, &rdata->SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO);
    
    glGetProgramInterfaceiv(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num_resources);
    glGetProgramInterfaceiv(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);

    for (GLint i = 0; i < num_resources; ++i) {
        GLenum prop = GL_NUM_ACTIVE_VARIABLES;
        GLint num_var, *vars, *offsets;
        GLchar **var_names;

        glGetProgramResourceName(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, i, ssbo_max_len, &string_len, resource_name);
        GLuint resource_index = glGetProgramResourceIndex(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, resource_name);
        log__write(log_handle, LOG_INFO, "SSBO Resource name %i: %s, index = %u", i, resource_name, resource_index);

        glGetProgramResourceiv(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, 1, NULL, &num_var);
        ERROR_CHECK(!(vars = malloc(num_var*sizeof(GLint))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
        prop = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(rdata->compute_program, GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, num_var, NULL, vars);
        log__write(log_handle, LOG_INFO, "SSBO Resource variables count = %i", num_var);
        for (GLint j = 0; j < num_var; ++j)
            log__write(log_handle, LOG_INFO, "  Resource variable = %i", vars[j]);

        ERROR_CHECK(!(offsets = malloc(num_var*sizeof(GLint))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
        ERROR_CHECK(!(var_names = malloc(num_var*sizeof(GLchar*))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
        prop = GL_OFFSET;
        for (GLint j = 0; j < num_var; ++j) {
            ERROR_CHECK(!(var_names[j] = malloc(32*sizeof(GLchar))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed at j = %i", j);
            glGetProgramResourceiv(rdata->compute_program, GL_BUFFER_VARIABLE, vars[j], 1, &prop, num_var, NULL, offsets);
            glGetProgramResourceName(rdata->compute_program, GL_BUFFER_VARIABLE, vars[j], 32, &string_len, var_names[j]);
            log__write(log_handle, LOG_INFO, "Variable %s offset = %i", var_names[j], offsets[j]);
        }

        free(vars);
        for (GLint j = 0; j < num_var; ++j)
            free(var_names[j]);
        free(var_names);

    }

    glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_BINDING_POINT, rdata->SSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, SSBO_SAMPLE_PERIOD_OFFSET, sizeof(float), &sample_period);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLES_OFFSET, sizeof(particle_t)*(P_COUNT+E_COUNT), particle_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void uniform_buffer_init(struct render_data_s *rdata)
{
    glGenBuffers(1, &rdata->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, rdata->UBO);
    glBufferData(GL_UNIFORM_BUFFER, UBO_SIZE, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING_POINT, rdata->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void vertex_array_object_init(struct render_data_s *rdata)
{
    glGenVertexArrays(VBO_COUNT, rdata->VAO);
    bind_vertex_array(rdata);
}

void bind_vertex_array(const struct render_data_s *rdata)
{
    for (buffer_index_e buf = P_BUF; buf <= E_BUF; ++buf) {

        glBindVertexArray(rdata->VAO[buf]);
        glBindVertexBuffer(buf, rdata->VBO[buf], 0, sizeof(struct vertex));
        glEnableVertexAttribArray(VS_IN_POS_LOC);
        glVertexAttribFormat(VS_IN_POS_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, pos));
        glVertexAttribBinding(VS_IN_POS_LOC, buf);
        glEnableVertexAttribArray(VS_IN_COLOR_LOC);
        glVertexAttribFormat(VS_IN_COLOR_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, color));
        glVertexAttribBinding(VS_IN_COLOR_LOC, buf);
    }

    glBindVertexArray(0);
}

void render_particles(const struct render_data_s *rdata, const unsigned int particle_index)
{
    glUseProgram(rdata->program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_OFFSET, sizeof(unsigned int), &particle_index);
    glBindBuffer(GL_UNIFORM_BUFFER, rdata->UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, UBO_VIEW_SCALAR_OFFSET, 4, &rdata->view_scalar);
    glBufferSubData(GL_UNIFORM_BUFFER, UBO_VIEW_RATIO_OFFSET, 4, &rdata->ratio);
    glBindVertexArray(rdata->VAO[particle_index<P_COUNT?P_BUF:E_BUF]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, rdata->num_segments);
    glBindVertexArray(0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void run_time_evolution_shader(const struct render_data_s *rdata, const unsigned int particle_index)
{
    glUseProgram(rdata->compute_program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_OFFSET, 4, &particle_index);
    glDispatchCompute(3, 3, 3);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color)
{
    for(unsigned int a = 0; a < num_segments; ++a) {
        const float theta = 2.0f * PI * a / num_segments;

        const float x = r * cos(theta);
        const float y = r * sin(theta);

        v[a].pos.i = x + center.i;
        v[a].pos.j = y + center.j;
        v[a].color = color;
    }
}

void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const float r, const int num_y_segments, const int num_z_segments, const color_t color)
{
    v[0].pos.i = 0 + center.i;
    v[0].pos.j = 0 + center.j;
    v[0].pos.k = r + center.k;
    v[0].color = color;

    for (unsigned int a = 0; a < num_z_segments; ++a) {

        const float phi = PI * (a+1) / num_z_segments;

        for(unsigned int b = 0; b < num_y_segments; ++b) {

            const unsigned int index = (num_z_segments * a) + b + 1;

            const float theta = 2.0 * PI * b / num_y_segments;

            const float x = r * cos(theta) * sin(phi);
            const float y = r * sin(theta) * sin(phi);
            const float z = r * cos(phi);
            
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
