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
    char *vs_text;
    char *fs_text;
    size_t rc;

    GLuint vertex_shader, fragment_shader;


    if (!vs_fp || !fs_fp) {
        return 1;
    }

    const size_t vs_size = get_file_size(vs_fp);
    const size_t fs_size = get_file_size(fs_fp);

    vs_text = malloc(sizeof(char)*vs_size);
    fs_text = malloc(sizeof(char)*fs_size);

    if (!vs_text || !fs_text) {
        return 1;
    }

    rc = fread(vs_text, sizeof(char), vs_size, vs_fp);
    if (rc != vs_size) {
        log__write(log_handle, WARNING, "VERTEX SHADER READ: read %i bytes\n\n%s\n", rc, vs_text);
        return 1;
    }
    rc = fread(fs_text, sizeof(char), fs_size, fs_fp);
    if (rc != fs_size) {
        log__write(log_handle, WARNING, "FRAGMENT SHADER READ: read %i bytes\n\n%s\n", rc, fs_text);
        return 1;
    }

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

    return 0;
}

void vertex_buffer_init(GLuint *VBO, const struct vertex *vertices, const int v_size)
{
    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, v_size, vertices, GL_STATIC_DRAW);
}

void vertex_buffer_draw(const GLuint VBO, const struct shader_variables shader_vars, const struct draw_variables draw_vars, const vector3d_t pos)
{
    mat4x4 m, p, mvp;

    mat4x4_translate(m, pos.i, pos.j, pos.k);
    mat4x4_scale(m, m, draw_vars.view_scalar);
    mat4x4_rotate_X(m, m, glfwGetTime());
    mat4x4_ortho(p, -draw_vars.ratio, draw_vars.ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /**
     * These attributes need to be reassociated with the currently bound vertex buffer object for the shader
     * to properly set the variables.
     */
    glEnableVertexAttribArray((GLuint)shader_vars.vpos_location);
    glVertexAttribPointer((GLuint)shader_vars.vpos_location, 3, GL_DOUBLE, GL_FALSE, sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray((GLuint)shader_vars.vcol_location);
    glVertexAttribPointer((GLuint)shader_vars.vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) (sizeof(double) * 3));

    glUniformMatrix4fv(shader_vars.mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
    glDrawArrays(GL_TRIANGLE_FAN, 0, draw_vars.num_segments);
}

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const double r, const int num_segments, const color_t color)
{
    for(int i = 0; i < num_segments; ++i) {
        double theta = 2.0f * PI * i / num_segments;

        double x = r * cos(theta);
        double y = r * sin(theta);

        v[i].pos.i = x + center.i;
        v[i].pos.j = y + center.j;
        v[i].color = color;
    }
}

void create_sphere_vertex_array(struct vertex *v, const vector3d_t center, const double r, const int num_y_segments, const int num_z_segments, const color_t color)
{
    /**
     * Drawing when rotating about the X-axis is still "fuzzy".
     */

    for (int a = 0; a < num_z_segments; ++a) {
        const double phi = PI * a / num_z_segments;

        for(int b = 0; b < num_y_segments; ++b) {
            const double theta = 2.0 * PI * b / num_y_segments;

            const double x = r * cos(theta) * cos(phi);
            const double y = r * sin(theta);
            const double z = r * cos(theta) * sin(phi);

            const int index = (num_z_segments * a) + b;

            v[index].pos.i = x + center.i;
            v[index].pos.j = y + center.j;
            v[index].pos.k = z + center.k;
            v[index].color = color;
        }
    }
}
