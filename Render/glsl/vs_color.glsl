#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
out vec4 world_position;
out vec4 world_normal;
out vec4 world_color;
out vec4 mvp_position;
out vec3 Normal;

out vec3 FragPos;



void main()
{
    world_position = model_matrix * vec4(position, 1.0f);
    world_normal = view_matrix * model_matrix * vec4(normal, 0.0f);
    world_normal = normalize(world_normal);
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
    mvp_position = gl_Position;
    world_color = vec4(color, 1.0f);
    FragPos = vec3(model_matrix * vec4(position, 1.0f));
    
    Normal = mat3(transpose(inverse(model_matrix))) * normal;  
}