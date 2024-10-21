#include "graphic_helpers.h"

#include <stdlib.h>

#include "particle_sim.h"
#include "particle.h"
#include "common.h"


#define DO_NOT__LOG_BLOCK_INTERFACE_ALIGNMENTS

#define SHADER_COUNT    3
#define COMPUTE_SHADER_FILEPATH     "shaders/time_evolution.comp.glsl"
#define VERTEX_SHADER_FILEPATH      "shaders/rasterization.vert.glsl"
#define FRAGMENT_SHADER_FILEPATH    "shaders/rasterization.frag.glsl"

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


/* Private function prototypes */
static void vertex_buffer_init(struct render_data_s *rdata, const buffer_index_e buf, void *data);
static void shader_storage_buffer_init(struct render_data_s *rdata, void *particles);
static void uniform_buffer_init(struct render_data_s *rdata);
static void bind_vertex_array(struct render_data_s *rdata);
static GLchar *shader_type_to_name(const enum shader_e type);
static GLchar *program_type_to_name(const program_e type);
static GLchar *get_shader_text(const enum shader_e type, size_t *text_len);
static GLuint check_shader_build_status(const enum shader_build_e stage, const GLuint obj_ref);
static GLuint compile_shader(const enum shader_e type);
static GLuint compile_shader_spir_v(const enum shader_e type);
static GLuint link_shader(const GLuint *shaders, const size_t s_size);
static void get_program_buffer_block_names_and_offsets(const GLuint program, const GLenum buf_interface, const enum shader_e s);
static void get_ssbo_info(const struct render_data_s *rdata, struct ssbo_info_s *info);
static void get_ubo_info(const struct render_data_s *rdata, struct ubo_info_s *info);


/* Public function definitions */
int shader_compile_and_link(struct render_data_s *rdata)
{
    GLuint shaders[SHADER_COUNT];
    int retval = EXIT_SUCCESS;

    /* get the shader source and compile */
    for (enum shader_e s = COMP; s <= FRAG; ++s)
        ERROR_CHECK(!(shaders[s]=compile_shader(s)),
                    exit(EXIT_FAILURE), LOG_ERROR, "Compiling %s shader failed.", shader_type_to_name(s));

    /* link programs */
    for (program_e p = COMP; p <= RASTER; ++p)
        ERROR_CHECK(!(rdata->program[p]=link_shader(&shaders[p==RASTER?VERT:COMP], p==RASTER?SHADER_COUNT-1:1)),
                    exit(EXIT_FAILURE), LOG_ERROR, "Linking %s shader failed.", program_type_to_name(p));
    
    /* clean up shader objects */
    glDetachShader(rdata->program[TIME_EVOLVE], shaders[COMP]);
    glDeleteShader(shaders[COMP]);
    for (enum shader_e s = VERT; s <= FRAG; ++s) {
        glDetachShader(rdata->program[RASTER], shaders[s]);
        glDeleteShader(shaders[s]);
    }

    #ifdef __LOG_BLOCK_INTERFACE_ALIGNMENTS
    get_program_buffer_block_names_and_offsets(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, COMP);
    get_program_buffer_block_names_and_offsets(rdata->program[TIME_EVOLVE], GL_UNIFORM_BLOCK, COMP);
    #endif

    get_ssbo_info(rdata, ssbo_info);
    get_ubo_info(rdata, &ubo_info);

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

void buffer_objects_init(struct render_data_s *rdata, void *p_verts, void *e_verts, void *particles)
{
    vertex_buffer_init(rdata, P_BUF, p_verts);
    vertex_buffer_init(rdata, E_BUF, e_verts);
    shader_storage_buffer_init(rdata, particles);
    uniform_buffer_init(rdata);
}

void vertex_array_object_init(struct render_data_s *rdata)
{
    glGenVertexArrays(VBO_COUNT, rdata->VAO);
    bind_vertex_array(rdata);
}

void set_particle_ssbo_data(const struct render_data_s *rdata, const GLenum access, void *particles)
{
    void *mapped_data;
    
    mapped_data = glMapNamedBuffer(rdata->SSBO[PARTICLE_SSBO], access);
    switch (access) {
        case GL_WRITE_ONLY:
            memcpy(mapped_data, particles, NUM_PARTICLES*ssbo_info[PARTICLE_SSBO].size);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            break;
            break;
        case GL_READ_ONLY:
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            memcpy(particles, mapped_data, NUM_PARTICLES*ssbo_info[PARTICLE_SSBO].size);
            break;
        default:
            log__write(log_handle, LOG_ERROR, "set_ssbo_data() called with invalid 'access'");
            exit(EXIT_FAILURE);
            break;
    }
    glUnmapNamedBuffer(rdata->SSBO[PARTICLE_SSBO]);
}

void render_particles(struct render_data_s *rdata, const unsigned int index, particle_t *particles)
{
    glUseProgram(rdata->program[RASTER]);
    glBindVertexArray(rdata->VAO[index<P_COUNT?P_BUF:E_BUF]);
    glUniform1ui(VERTEX_INDEX_LOC, index);
    glDrawArrays(GL_TRIANGLE_FAN, 0, rdata->num_segments);
    glBindVertexArray(0);

}

void run_time_evolution_shader(struct render_data_s *rdata, particle_t *particles)
{
    glUseProgram(rdata->program[TIME_EVOLVE]);
    glNamedBufferSubData(rdata->UBO, ubo_info.view_scalar_offset, sizeof(float), &rdata->view_scalar);
    glNamedBufferSubData(rdata->UBO, ubo_info.ratio_offset, sizeof(float), &rdata->ratio);
    glDispatchCompute(NUM_PARTICLES, 1, 1);
    set_particle_ssbo_data(rdata, GL_READ_ONLY, particles);
}

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color)
{
    for(unsigned int a = 0; a < num_segments; ++a) {
        const float theta = 2.0f * PI * a / num_segments;

        const float x = r * cosf(theta);
        const float y = r * sinf(theta);

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

            const float x = r * cosf(theta) * sinf(phi);
            const float y = r * sinf(theta) * sinf(phi);
            const float z = r * cosf(phi);
            
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


/* Private function definitions */
static void vertex_buffer_init(struct render_data_s *rdata, const buffer_index_e buf, void *data)
{
    glGenBuffers(1, &rdata->VBO[buf]);
    glBindBuffer(GL_ARRAY_BUFFER, rdata->VBO[buf]);
    glNamedBufferData(rdata->VBO[buf], NUM_SEGMENTS*sizeof(struct vertex), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void shader_storage_buffer_init(struct render_data_s *rdata, void *particles)
{
    glGenBuffers(SSBO_COUNT, rdata->SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO[PARTICLE_SSBO]);
    glNamedBufferStorage(rdata->SSBO[PARTICLE_SSBO], NUM_PARTICLES*ssbo_info[PARTICLE_SSBO].size, particles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_info[PARTICLE_SSBO].binding, rdata->SSBO[PARTICLE_SSBO]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdata->SSBO[MVP_SSBO]);
    glNamedBufferStorage(rdata->SSBO[MVP_SSBO], NUM_PARTICLES*ssbo_info[MVP_SSBO].size, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_info[MVP_SSBO].binding, rdata->SSBO[MVP_SSBO]);
}

static void uniform_buffer_init(struct render_data_s *rdata)
{
    glGenBuffers(1, &rdata->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, rdata->UBO);
    glNamedBufferData(rdata->UBO, ubo_info.size, NULL, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(rdata->UBO, ubo_info.sample_period_offset, sizeof(float), &sample_period);
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_info.binding, rdata->UBO);
}

static void bind_vertex_array(struct render_data_s *rdata)
{
    for (buffer_index_e buf = P_BUF; buf <= E_BUF; ++buf) {

        glBindVertexArray(rdata->VAO[buf]);
        glBindVertexBuffer(buf, rdata->VBO[buf], 0, sizeof(struct vertex));
        glEnableVertexAttribArray(VERTEX_POS_LOC);
        glVertexAttribFormat(VERTEX_POS_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, pos));
        glVertexAttribBinding(VERTEX_POS_LOC, buf);
        glEnableVertexAttribArray(VERTEX_COLOR_LOC);
        glVertexAttribFormat(VERTEX_COLOR_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, color));
        glVertexAttribBinding(VERTEX_COLOR_LOC, buf);
    }

    glBindVertexArray(0);
}

static GLchar *shader_type_to_name(const enum shader_e type)
{
    switch (type) {
        case COMP: return "COMPUTE"; break;
        case VERT: return "VERTEX"; break;
        case FRAG: return "FRAGMENT"; break;
        default: return "WRONG_TYPE"; break;
    }
}

static GLchar *program_type_to_name(const program_e type)
{
    switch (type) {
        case TIME_EVOLVE: "TIME_EVOLUTION"; break;
        case RASTER: "RASTERIZATION"; break;
        default: "WRONG_TYPE"; break;
    }
}

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
    
    if (file_size != fread(text, sizeof(char), file_size, fp)) {
        log__write(log_handle, LOG_WARNING, "%s SHADER READ: read incorrect bytes\n\n%s\n", shader_type_to_name(type), text);
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

static void get_program_buffer_block_names_and_offsets(const GLuint program, const GLenum buf_interface, const enum shader_e s)
{
    GLint num_resources, bo_max_len, var_max_len;
    GLchar *resource_name, *var_name;

    /* get the number of bos, the max length of the bo names, and the max length of variable names */
    glGetProgramInterfaceiv(program, buf_interface, GL_ACTIVE_RESOURCES, &num_resources);
    glGetProgramInterfaceiv(program, buf_interface, GL_MAX_NAME_LENGTH, &bo_max_len);
    glGetProgramInterfaceiv(program, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);
    ERROR_CHECK(!(resource_name=malloc(bo_max_len*sizeof(GLchar)+1)), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
    ERROR_CHECK(!(var_name=malloc(var_max_len*sizeof(GLchar)+1)), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");

    for (GLint i = 0; i < num_resources; ++i) {
        GLenum prop;
        GLint num_var, *vars, offset, buffer_data_size;

        /* Get the bo name */
        glGetProgramResourceName(program, buf_interface, i, bo_max_len, NULL, resource_name);

        /* get the resource index */
        GLuint resource_index = glGetProgramResourceIndex(program, buf_interface, resource_name);
        log__write(log_handle, LOG_INFO, "%s shader's resource name: %s", shader_type_to_name(s), resource_name);

        /* get the number of variables in the bo */
        prop = GL_NUM_ACTIVE_VARIABLES;
        glGetProgramResourceiv(program, buf_interface, resource_index, 1, &prop, 1, NULL, &num_var);
        ERROR_CHECK(!(vars=malloc(num_var*sizeof(GLint))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
        
        /* get the active variables within the bo */
        prop = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(program, buf_interface, resource_index, 1, &prop, num_var, NULL, vars);

        /* get and print to log the names and offsets for the bo variables */
        prop = GL_OFFSET;
        for (GLint j = 0; j < num_var; ++j) {
            glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, vars[j], 1, &prop, num_var, NULL, &offset);
            glGetProgramResourceName(program, GL_BUFFER_VARIABLE, vars[j], var_max_len, NULL, var_name);
            log__write(log_handle, LOG_INFO, "  Variable '%s' offset = %u", var_name, offset);
        }

        free(vars);

        prop = GL_BUFFER_DATA_SIZE;
        glGetProgramResourceiv(program, buf_interface, resource_index, 1, &prop, 1, NULL, &buffer_data_size);
        log__write(log_handle, LOG_INFO, "  Buffer data size = %u", buffer_data_size);
    }

    free(resource_name);
    free(var_name);
}

static void get_ssbo_info(const struct render_data_s *rdata, struct ssbo_info_s *info)
{
    GLint num_resources, ssbo_max_len, var_max_len;
    GLchar *resource_name, *var_name;

    /* get the number of ssbos, the max length of the ssbo names, and the max length of variable names */
    glGetProgramInterfaceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num_resources);
    glGetProgramInterfaceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);
    glGetProgramInterfaceiv(rdata->program[TIME_EVOLVE], GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);
    ERROR_CHECK(!(resource_name=malloc(ssbo_max_len*sizeof(GLchar)+1)), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
    ERROR_CHECK(!(var_name=malloc(var_max_len*sizeof(GLchar)+1)), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");

    for (GLint i = 0; i < num_resources; ++i) {
        GLenum prop;
        GLint num_var, *vars, offset, buffer_data_size;

        /* Get the ssbo name */
        prop = GL_NUM_ACTIVE_VARIABLES;
        glGetProgramResourceName(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, i, ssbo_max_len, NULL, resource_name);

        /* get the resource index */
        GLuint resource_index = glGetProgramResourceIndex(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, resource_name);

        /* get the number of variables in the ssbo */
        glGetProgramResourceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, 1, NULL, &num_var);
        ERROR_CHECK(!(vars=malloc(num_var*sizeof(GLint))), exit(EXIT_FAILURE), LOG_ERROR, "malloc() failed");
        
        /* get the active variables within the ssbo */
        prop = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, num_var, NULL, vars);

        /* get and print to log the names and offsets for the ssbo variables */
        prop = GL_OFFSET;
        for (GLint j = 0; j < num_var; ++j) {
            glGetProgramResourceiv(rdata->program[TIME_EVOLVE], GL_BUFFER_VARIABLE, vars[j], 1, &prop, num_var, NULL, &offset);
            glGetProgramResourceName(rdata->program[TIME_EVOLVE], GL_BUFFER_VARIABLE, vars[j], var_max_len, NULL, var_name);

            if (!strcmp(resource_name, "particle_data_block") && !strcmp(var_name, "particles[0].id")) {
                info[PARTICLE_SSBO].binding = 0;
                info[PARTICLE_SSBO].offset = offset;
            } else if (!strcmp(resource_name, "MVP_data_block") && !strcmp(var_name, "MVP[0]")) {
                info[MVP_SSBO].binding = 1;
                info[MVP_SSBO].offset = offset;
            }
       }

        free(vars);

        prop = GL_BUFFER_DATA_SIZE;
        if (!strcmp(resource_name, "particle_data_block")) {
            glGetProgramResourceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, 1, NULL, &info[PARTICLE_SSBO].size);
        } else if (!strcmp(resource_name, "MVP_data_block")) {
            glGetProgramResourceiv(rdata->program[TIME_EVOLVE], GL_SHADER_STORAGE_BLOCK, resource_index, 1, &prop, 1, NULL, &info[MVP_SSBO].size);
        }
    }

    free(resource_name);
    free(var_name);
}

static void get_ubo_info(const struct render_data_s *rdata, struct ubo_info_s *info)
{
    GLuint block_index = glGetUniformBlockIndex(rdata->program[TIME_EVOLVE], "uniform_data_block");
    glGetActiveUniformBlockiv(rdata->program[TIME_EVOLVE], block_index, GL_UNIFORM_BLOCK_BINDING, &info->binding);
    glGetActiveUniformBlockiv(rdata->program[TIME_EVOLVE], block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &info->size);

    GLuint individual_var_offset = info->size / 3;
    info->sample_period_offset = 0;
    info->view_scalar_offset = info->sample_period_offset + individual_var_offset;
    info->ratio_offset = info->view_scalar_offset + individual_var_offset;
}
