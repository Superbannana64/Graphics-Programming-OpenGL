#version 450                          
out vec4 FragColor;

uniform vec3 _ObjectColor;
uniform vec3 _LightColor;
uniform vec3 _LightPos;
uniform vec3 _ViewPos;

in vec3 Normal;
in vec3 FragPos;

void main(){         

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * _LightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(_LightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * _LightColor;

    float specularStrength = 0.5f;

    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * _LightColor;  

    vec3 result = (ambient+diffuse+specular) * _ObjectColor;

    FragColor = vec4(result,1.0f);
}
