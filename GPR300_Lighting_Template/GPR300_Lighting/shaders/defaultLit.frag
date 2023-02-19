#version 450                          
out vec4 FragColor;

struct Material {
    vec3 objectColor;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
}; 

//Make different structs for directional, point, and spot lights
struct Light {
    vec3 position;
    vec3 lightColor;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 _LightColor;
uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform Material material;
uniform Light light;  

in vec3 Normal;
in vec3 FragPos;

void main(){         
    
    //ambient
    vec3 ambient = (_LightColor*light.ambient) * material.ambient;

    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(_LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = _LightColor*light.diffuse * (diff * material.diffuse);

    //specular
    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = _LightColor*light.specular * (spec * material.specular);    

    vec3 result = material.objectColor * (ambient+diffuse+specular);

    FragColor = vec4(result,1.0f);
}
