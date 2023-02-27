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

void main(){         
    
    vec3 Norm = normalize(Normal);
    vec3 viewDir = normalize(_ViewPos - FragPos);

    ///Directional Light
    //ambient
    /*
    vec3 dAmbient = (material.objectColor * material.ambient) * dLight.ambient * (dLight.lightColor * dLight.intensity);

    //Diffuse
    vec3 dLightDir = normalize(-dLight.direction);
    float dDiff = max(dot(Norm, dLightDir), 0.0);
    vec3 dDiffuse = dLight.lightColor*dLight.diffuse * (dDiff * material.diffuse);

    //specular
    vec3 dReflectDir = reflect(-dLightDir, Norm);
    float dSpec = pow(max(dot(viewDir, dReflectDir), 0.0), material.shininess);
    vec3 dSpecular = dLight.lightColor*dLight.specular * (dSpec * material.specular);    

    vec3 result = (dAmbient+dDiffuse+dSpecular);*/

    ///Point Light
    //Ambient
    /*
    vec3 pAmbient = (material.objectColor * material.ambient) * pLight.ambient * (pLight.lightColor * pLight.intensity);

    //Diffuse
    vec3 pLightDir = normalize(-pLight.position);
    float pDiff = max(dot(Norm, pLightDir), 0.0);
    vec3 pDiffuse = pLight.lightColor * pLight.diffuse * (pDiff * material.diffuse);

    //specular
    vec3 pReflectDir = reflect(-pLightDir, Norm);
    float pSpec = pow(max(dot(viewDir, pReflectDir), 0.0), material.shininess);
    vec3 pSpecular = pLight.lightColor*pLight.specular * (pSpec * material.specular);

    //Attenuation
    float distance = length(pLight.position - FragPos);
    float attenuation = 1.0 / (pLight.constant + pLight.linear * distance + pLight.quadratic * (distance * distance)); 

    pAmbient *= attenuation;
    pDiffuse *= attenuation;
    pSpecular *= attenuation;

    vec3 result = (pAmbient+pDiffuse+pSpecular);*/


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

    vec3 result = (sAmbient+sDiffuse+sSpecular);

    FragColor = vec4(result,1.0f);
}
