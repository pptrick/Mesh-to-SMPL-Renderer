#version 440
in vec4 segcolor;
out vec4 fcolor;

void main()
{
    float scale = 1;
    fcolor = segcolor * scale;
}