#version 400

uniform mat4 mvp;
uniform mat4 mv;
uniform vec3 lightPos = vec3(400.0f, 400.0f, -100.0f);
uniform vec3 location = vec3(0.0f, 0.0f, 0.0f);

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 aTexCod;


out vec2 vTexCod;
out vec3 norm_eye;
out vec3 light_eye;
out vec3 view_eye;
    

void main()
{
    
    float x,y,z;
    x = VertexPosition.x + location.x; 
    y = VertexPosition.y + location.y; 
    z = VertexPosition.z + location.z; 
        
    vec4 newPos = vec4(x, y, z, 1.0);
    gl_Position = mvp  * newPos;
    vTexCod = aTexCod;
    norm_eye = (mv * vec4(normal, 1)).xyz;
    vec3 pos_eye = (mv * vec4(VertexPosition, 1)).xyz;
    
    
    vec3 lightPos_eye = (mv * vec4(lightPos, 1.0f)).xyz;
    light_eye = lightPos_eye - pos_eye.xyz;
    view_eye = vec3(pos_eye);

    light_eye = normalize(light_eye);
    view_eye  = normalize(view_eye);
}

