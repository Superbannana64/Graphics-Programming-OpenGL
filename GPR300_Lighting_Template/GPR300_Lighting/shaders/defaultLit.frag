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
struct DirectionalLight {
    vec3 direction;
    vec3 lightColor;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform Material material;
uniform DirectionalLight dLight;  

in vec3 Normal;
in vec3 FragPos;

void main(){         
    
    //ambient
    vec3 ambient = (dLight.lightColor*dLight.ambient) * material.ambient;

    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-dLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = dLight.lightColor*dLight.diffuse * (diff * material.diffuse);

    //specular
    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = dLight.lightColor*dLight.specular * (spec * material.specular);    

    vec3 result = material.objectColor * (ambient+diffuse+specular);

    FragColor = vec4(result,1.0f);
}
