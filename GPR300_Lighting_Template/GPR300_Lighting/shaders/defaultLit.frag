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
uniform PointLight pLight;
uniform SpotLight sLight;

in vec3 Normal;
in vec3 FragPos;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main(){         
    
    vec3 Norm = normalize(Normal);
    vec3 viewDir = normalize(_ViewPos - FragPos);

    vec3 result = CalcDirLight(dLight, Norm, viewDir);
    result += CalcPointLight(pLight, Norm,FragPos, viewDir);

    /*
    vec3 sLightDir = normalize(-sLight.position);
    //Spot Light
    float theta     = dot(sLightDir, normalize(-sLight.direction));
    float epsilon   = sLight.cutOff - sLight.outerCutOff;
    float intensity = clamp((theta - sLight.outerCutOff) / epsilon, 0.0, 1.0);

    //amb
    vec3 sAmbient = (material.objectColor * material.ambient) * sLight.ambient * (sLight.lightColor * sLight.intensity);

    //diffuse
    float sDiff = max(dot(Norm, sLightDir), 0.0);
    vec3 sDiffuse = sLight.lightColor * sLight.diffuse * (sDiff * material.diffuse);

    //specular
    vec3 sReflectDir = reflect(-sLightDir, Norm);
    float sSpec = pow(max(dot(viewDir, sReflectDir), 0.0), material.shininess);
    vec3 sSpecular = sLight.lightColor*sLight.specular * (sSpec * material.specular);

    //Attenuation
    float distance = length(sLight.position - FragPos);
    float attenuation = 1.0 / (sLight.constant + sLight.linear * distance + sLight.quadratic * (distance * distance)); 

    sAmbient *= attenuation;
    sDiffuse *= attenuation;
    sSpecular *= attenuation;

    sDiffuse *= intensity;
    sSpecular *= intensity;

    vec3 result = (sAmbient+sDiffuse+sSpecular);*/

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
    // combine results
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 