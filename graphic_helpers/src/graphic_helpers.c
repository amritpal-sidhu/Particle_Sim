#include "graphic_helpers.h"

#include <stdlib.h>

#include "particle.h"
#include "common.h"


#define SHADER_COUNT    2
#define VERTEX_SHADER_FILEPATH      "shaders/vs.vert.glsl"
#define FRAGMENT_SHADER_FILEPATH    "shaders/fs.frag.glsl"

enum shader_e {
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
    VERTEX_SHADER_FILEPATH,
    FRAGMENT_SHADER_FILEPATH
};

static const  GLenum shader_type[SHADER_COUNT] = {
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

    if (file_size != fread(text, sizeof(char), file_size, fp)) {
        log__write(log_handle, LOG_WARNING, "%s SHADER READ: read incorrect bytes\n\n%s\n", type==VERT?"VERTEX":"FRAGMENT", text);
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

static GLuint link_shader(const GLuint *shaders)
{
    GLuint program = glCreateProgram();
    for (size_t i = 0; i < SHADER_COUNT; ++i)
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
    for (enum shader_e s = VERT; s <= FRAG; ++s) {
        if (!(shaders[s]=compile_shader(s)))
            return EXIT_FAILURE;
    }

    /* link program */
    if (!(rdata->program=link_shader(shaders)))
        retval = EXIT_FAILURE;
    
    /* clean up shader objects */
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
        glEnableVertexAttribArray(POS_ATTR_IDX);
        glEnableVertexAttribArray(COL_ATTR_IDX);
        glVertexAttribLFormat(POS_ATTR_IDX, 3, GL_DOUBLE, offsetof(struct vertex, pos));
        glVertexAttribFormat(COL_ATTR_IDX, 3, GL_FLOAT, GL_FALSE, offsetof(struct vertex, color));
        glVertexAttribBinding(POS_ATTR_IDX, buf);
        glVertexAttribBinding(COL_ATTR_IDX, buf);
        glBindVertexBuffer(buf, 0, 0, 0);
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

void update_mvp_uniform(struct render_data_s *rdata, const vector3d_t pos, const vector3d_t angle)
{
    mat4x4 mvp, m, p;

    mat4x4_translate(m, pos.i, pos.j, pos.k);
    /* Angles need to be "unscaled" */
    mat4x4_rotate_X(m, m, angle.i/rdata->view_scalar);
    mat4x4_rotate_Y(m, m, angle.j/rdata->view_scalar);
    mat4x4_rotate_Z(m, m, angle.k/rdata->view_scalar);
    mat4x4_scale(m, m, rdata->view_scalar);
    mat4x4_ortho(p, -rdata->ratio, rdata->ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);

    glUniformMatrix4fv(MVP_UNION_IDX, 1, GL_FALSE, (const GLfloat *)mvp);
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
