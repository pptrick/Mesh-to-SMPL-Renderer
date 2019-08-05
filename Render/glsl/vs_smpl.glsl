#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec4 segcolor;


uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    segcolor = vec4(color, 1.0);
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
}