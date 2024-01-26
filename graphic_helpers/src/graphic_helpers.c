#include "graphic_helpers.h"

#include <stdlib.h>

#include "particle.h"
#include "log.h"


/* Global variables */
extern log_t *log_handle;

const color_t p_color = (color_t){.r = 1.0f, .g = 0.0f, .b = 0.0f};
const color_t e_color = (color_t){.r = 0.0f, .g = 0.0f, .b = 1.0f};;


/* Public function definitions */
int shader_compile_and_link(GLuint *program)
{
    FILE *vs_fp = fopen(VERTEX_SHADER_FILEPATH, "r");
    FILE *fs_fp = fopen(FRAGMENT_SHADER_FILEPATH, "r");
    char *vs_text, *fs_text;
    GLuint vertex_shader, fragment_shader;
    int status;
    size_t rc;


    if (!vs_fp || !fs_fp) {
        return 1;
    }

    const size_t vs_size = get_file_size(vs_fp);
    const size_t fs_size = get_file_size(fs_fp);

    vs_text = malloc(sizeof(char)*vs_size+1);
    fs_text = malloc(sizeof(char)*fs_size+1);

    if (!vs_text || !fs_text) {
        return 1;
    }

    rc = fread(vs_text, sizeof(char), vs_size, vs_fp);
    if (rc != vs_size) {
        log__write(log_handle, LOG_WARNING, "VERTEX SHADER READ: read %i bytes\n\n%s\n", rc, vs_text);
        return 1;
    }
    vs_text[sizeof(char)*vs_size] = '\0';
    
    rc = fread(fs_text, sizeof(char), fs_size, fs_fp);
    if (rc != fs_size) {
        log__write(log_handle, LOG_WARNING, "FRAGMENT SHADER READ: read %i bytes\n\n%s\n", rc, fs_text);
        return 1;
    }
    fs_text[sizeof(char)*fs_size] = '\0';

    fclose(vs_fp);
    fclose(fs_fp);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar * const*)&vs_text, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char info[512];
        glGetShaderInfoLog(vertex_shader, sizeof(info), NULL, info);
        log__write(log_handle, LOG_ERROR, "%s", info);
        log__write(log_handle, LOG_ERROR, "SHADER TEXT DUMP (%i): %s", vs_size, vs_text);
        return 1;
    }
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar * const*)&fs_text, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char info[512];
        glGetShaderInfoLog(fragment_shader, sizeof(info), NULL, info);
        log__write(log_handle, LOG_ERROR, "%s", info);
        return 1;
    }
 
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);

    /* bind data and attributes */
    glBindAttribLocation(*program, VERT_POS_LOC, VERT_POS_VAR);
    glBindAttribLocation(*program, VERT_COL_LOC, VERT_COL_VAR);
    
    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &status);
    if (!status) {
        char info[512];
        glGetShaderInfoLog(fragment_shader, sizeof(info), NULL, info);
        log__write(log_handle, LOG_ERROR, "%s", info);
        return 1;
    }

    free(vs_text);
    free(fs_text);
    glDetachShader(*program, vertex_shader);
    glDeleteShader(vertex_shader);
    glDetachShader(*program, fragment_shader);
    glDeleteShader(fragment_shader);

    return 0;
}

void enable_vertex_attributes(void)
{
    const GLsizei stride = sizeof(struct vertex);
    const GLintptr position_offset = 0;
    const GLintptr color_offset = sizeof(vector3d_t);

    glEnableVertexAttribArray(VERT_POS_LOC);
    glEnableVertexAttribArray(VERT_COL_LOC);

    glVertexAttribLPointer(VERT_POS_LOC, 3, GL_DOUBLE, stride, (GLvoid*)position_offset);
    glVertexAttribPointer(VERT_COL_LOC, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)color_offset);
}

void vertex_array_object_init(GLuint *VAO, GLuint *VBO, const size_t VBO_count)
{
    glGenVertexArrays(VBO_count, VAO);

    for (size_t i = 0; i < VBO_count; ++i)
    {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        enable_vertex_attributes();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size)
{
    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, v_size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vertex_buffer_draw(const GLuint program, const struct draw_variables draw_vars)
{
    mat4x4 m, p, mvp;

    mat4x4_translate(m, draw_vars.pos.i, draw_vars.pos.j, draw_vars.pos.k);
    /* Angles need to be "unscaled" */
    mat4x4_rotate_X(m, m, draw_vars.angle.i/draw_vars.view_scalar);
    mat4x4_rotate_Y(m, m, draw_vars.angle.j/draw_vars.view_scalar);
    mat4x4_rotate_Z(m, m, draw_vars.angle.k/draw_vars.view_scalar);
    mat4x4_scale(m, m, draw_vars.view_scalar);
    mat4x4_ortho(p, -draw_vars.ratio, draw_vars.ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);

    glUniformMatrix4fv(glGetUniformLocation(program, UNIFORM_MVP_VAR), 1, GL_FALSE, (const GLfloat *)mvp);
    glDrawArrays(GL_TRIANGLE_FAN, 0, draw_vars.num_segments);
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
