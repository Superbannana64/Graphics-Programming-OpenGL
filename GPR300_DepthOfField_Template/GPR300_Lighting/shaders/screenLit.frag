#version 450                          
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool invCol;

void main()
{
    if(invCol)
    {
        //Inverse Colors
        FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
    }
    else
    {
        vec3 col = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(col, 1.0);
    }
} 