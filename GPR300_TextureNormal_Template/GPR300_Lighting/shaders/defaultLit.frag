#version 450                          
out vec4 FragColor;

struct Material {
    vec3 objectColor;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
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

in vec3 Normal;
in vec2 TexCoord1;
in vec3 FragPos;
in mat3 TBN;

uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform Material material;

uniform sampler2D ourTexture;
uniform sampler2D ourNormTexture;

uniform PointLight pLight;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main(){         
    

    vec3 Norm = texture(ourNormTexture, TexCoord1).rgb;
    Norm = normalize(Norm*2.0 - 1.0);

    Norm = normalize(TBN * Norm);

    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 result;
    result += CalcPointLight(pLight, Norm,FragPos, viewDir);

    //FragColor = vec4(result,1.0f);
    FragColor = texture(ourTexture, TexCoord1) * vec4(result,1.0);
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