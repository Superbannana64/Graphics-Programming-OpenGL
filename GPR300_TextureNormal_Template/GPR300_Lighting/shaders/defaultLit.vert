#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 TexCoord;
layout (location = 3) in vec3 aTangent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out vec3 Normal;
out vec2 TexCoord1;
out vec3 FragPos;

void main(){    
    
    Normal = vNormal;
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
    TexCoord1 = TexCoord;
    FragPos = vec3(_Model * vec4(vPos,1.0));
}
