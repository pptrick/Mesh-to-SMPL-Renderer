#version 440

out vec4 fcolor;
in vec4 world_position;
in vec4 world_normal;
in vec4 world_color;
in vec4 mvp_position;
in vec3 Normal;
in vec3 FragPos;
uniform int render_depth;

uniform vec3 lightPos; 

uniform vec3 viewPos;

uniform vec3 lightColor;

void main()
{
    float scale = 1;
    vec3 BufferColor = vec3(world_color.x,world_color.y,world_color.z);
    //fcolor = world_color * scale;

    // Ambient
    
    float ambientStrength = 0.2f;
    
    vec3 ambient = ambientStrength * lightColor;
  	
    
    // Diffuse 
    
    vec3 norm = normalize(Normal);
    
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 diffuse = diff * lightColor * 1.7;
    
    
    // Specular
    
    float specularStrength = 0.6f;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 reflectDir = reflect(-lightDir, norm);  
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 19);
    
    vec3 specular = specularStrength * spec * lightColor; 
 
        
   
    vec3 result = (ambient + diffuse + specular) * BufferColor * scale;
    
    fcolor = vec4(result, 1.0f);


     if (render_depth == 1)
    {
        float color = mvp_position.z / mvp_position.w;
        color = color * 0.5 + 0.5;
        fcolor = vec4(color, color, color, 1.0);
    }

}