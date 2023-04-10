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

uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform Material material;
uniform DirectionalLight dLight; 
uniform sampler2D shadowMap;
uniform float minBias;
uniform float maxBias;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 fragPos); 
float ShadowCalculation(vec4 fragPosLightSpace, float bias);

void main(){         
    
    vec3 Norm = normalize(Normal);
    vec3 viewDir = normalize(_ViewPos - FragPos);

    vec3 result;
    result += CalcDirLight(dLight, Norm, viewDir, FragPos);
    FragColor = vec4(result,1.0f);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 lightDir = normalize(light.direction - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    vec3 ambient  = (material.objectColor * material.ambient) * light.ambient * (light.lightColor * light.intensity);
    vec3 diffuse  = light.lightColor*light.diffuse * (diff * material.diffuse);
    vec3 specular = light.lightColor*light.specular * (spec * material.specular);
    
    //min and max bias values (0.05 and 0.005 need to be in gui)
    float bias = max(maxBias * (1.0 - dot(normal, lightDir)), minBias);

    float shadow = ShadowCalculation(FragPosLightSpace, bias);

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * material.objectColor;

    return (lighting);
}
float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;



    return shadow;
}