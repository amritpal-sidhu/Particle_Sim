// Note to self regarding qualifiers:
//      uniform is per primative
//      attribute is per vertex
//      varying is per fragment, .vert to .frag

// Source: https://stackoverflow.com/questions/17537879/in-webgl-what-are-the-differences-between-an-attribute-a-uniform-and-a-varying

// #version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec3 vPos;
varying vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
}

