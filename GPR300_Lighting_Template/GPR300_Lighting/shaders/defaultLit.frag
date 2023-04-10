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
    float intensity;
};

struct PointLight {
    vec3 position;  
    vec3 lightColor;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float intensity;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight{
    vec3  position;
    vec3  direction;
    vec3 lightColor;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float intensity;

};

uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform Material material;

uniform DirectionalLight dLight;  

in vec3 Normal;
in vec3 FragPos;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main(){         
    
    vec3 Norm = normalize(Normal);
    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 result;

    result += CalcDirLight(dLight, Norm, viewDir);

    FragColor = vec4(result,1.0f);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
   
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    diffuse  *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);
}