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
        log__write(log_handle, LOG_WARNING, "VERTEX SHADER READ: read %i bytes\n\n%s\n", rc, vs_text);
        return 1;
    }
    rc = fread(fs_text, sizeof(char), fs_size, fs_fp);
    if (rc != fs_size) {
        log__write(log_handle, LOG_WARNING, "FRAGMENT SHADER READ: read %i bytes\n\n%s\n", rc, fs_text);
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

void vertex_buffer_draw(const GLuint VBO, const struct draw_variables draw_vars)
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


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /**
     * These attributes need to be reassociated with the currently bound vertex buffer object for the shader
     * to properly set the variables.
     */
    glEnableVertexAttribArray((GLuint)draw_vars.shader_vars.vpos_location);
    glVertexAttribPointer((GLuint)draw_vars.shader_vars.vpos_location, 3, GL_DOUBLE, GL_FALSE, sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray((GLuint)draw_vars.shader_vars.vcol_location);
    glVertexAttribPointer((GLuint)draw_vars.shader_vars.vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) (sizeof(double) * 3));

    glUniformMatrix4fv(draw_vars.shader_vars.mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
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
