#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord1;

uniform sampler2D ourTexture;

void main(){         
    //FragColor = vec4(abs(Normal),1.0f);

    FragColor = texture(ourTexture, TexCoord1);
    

}
