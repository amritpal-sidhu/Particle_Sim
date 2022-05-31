#include "graphic_helpers.h"

#include <stdlib.h>
#include <time.h>

#include "particle.h"


/* Private function declarations */
static size_t get_file_size(FILE *fp);


/* Global variables */
const double p_radius = 0.1;
const double e_radius = 0.05;
const color_t p_color = (color_t){.r = 1.0f, .g = 0.0f, .b = 0.0f};
const color_t e_color = (color_t){.r = 0.0f, .g = 0.0f, .b = 1.0f};;

FILE *debug_fp;


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

void create_circle_vertex_array(struct vertex *v, const vector2d_t center, const float r, const int num_segments, const color_t color)
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

void busy_wait_ms(const float delay_in_ms)
{
    const float clocks_per_ms = (float)CLOCKS_PER_SEC / 1000;
    const float start_tick = clocks_per_ms * clock();
    const float end_tick = start_tick + (clocks_per_ms * delay_in_ms);

    while (clocks_per_ms * clock() <= end_tick);
}


/* Private function definitions */
static size_t get_file_size(FILE *fp)
{
    size_t size;

    fseek(fp, 0, SEEK_END);
    size = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return size;
}
