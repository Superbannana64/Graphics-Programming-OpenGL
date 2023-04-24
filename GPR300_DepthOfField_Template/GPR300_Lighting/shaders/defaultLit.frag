#version 450                          
out vec4 FragColor;

struct Material {
    vec3 objectColor;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
}; 

struct DirectionalLight {
    vec3 direction;
    vec3 lightColor;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

uniform vec3 _LightPos;
uniform vec3 _ViewPos;

uniform DirectionalLight dLight;  
uniform Material material;

in vec3 Normal;
in vec2 TexCoord1;
in vec3 FragPos;

uniform sampler2D ourTexture;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);

void main(){         
    //FragColor = vec4(abs(Normal),1.0f);

    vec3 viewDir = normalize(_ViewPos - FragPos);
    vec3 result;
    result += CalcDirLight(dLight, Normal, viewDir);

    FragColor = texture(ourTexture, TexCoord1) * vec4(result,1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, -lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);

    return (ambient + diffuse + specular);
}
