#version 450                          
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D blurTexture;
uniform sampler2D depthTexture;

uniform float test;

//Make all gui stuff
uniform float focusDistance =  5.0f;
uniform float farDof = 1.0f;
uniform float nearDof = 1.0f;

//how quick does it fade to nothing, higher is quicker
uniform float falloff = 100.0f;

//note results vary wildly when you change these
uniform float nearPlane = 0.1f;
uniform float farPlane = 1000.0f;

uniform float minStrength = 0.05f;

float depthStrength()
{
    float distance = texture2D(depthTexture, TexCoords).x;

    float z = (2 * nearPlane) / (farPlane + nearPlane - distance * (farPlane - nearPlane));
    
    float dofStrength = 1;
    
    float sFarDof = (focusDistance+farDof)/farPlane;
    float sNearDof = (focusDistance-nearDof)/farPlane;    
     
    if(z > sFarDof)
    {
        dofStrength = ((sFarDof)/(z));
    }
    else if (z < sNearDof) {
       dofStrength = ((z)/(sNearDof));
    }    

    dofStrength = clamp(1-dofStrength,0.0f,1.0f);
    dofStrength *= falloff;
    dofStrength =clamp(1-dofStrength,0.0f,1.0f);
    return mix(minStrength, 1.0f,dofStrength);
}

void main()
{
    if(test == 1)
    {
        float dofStrength = depthStrength();
        vec3 texCol = mix(texture2D(screenTexture, TexCoords),texture2D(blurTexture, TexCoords),1-dofStrength).rgb;
        FragColor = vec4(texCol,1);
    }
    else
    {
        vec3 col;
        col = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(col, 1.0);
    }
} 