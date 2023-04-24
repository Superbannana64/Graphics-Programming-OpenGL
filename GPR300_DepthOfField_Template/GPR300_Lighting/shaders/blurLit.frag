#version 450                          
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float Intensity = 4.0f;

vec4 boxBlur() {
	vec2 texel = Intensity / textureSize(screenTexture, 0).xy;
	
    vec2 coord = clamp(TexCoords, texel, vec2(1,1) - (texel * 2));
    
	vec4 color;
	color += texture(screenTexture, coord + vec2(-texel.x,  texel.y));
	color += texture(screenTexture, coord + vec2(-texel.x,  0));
	color += texture(screenTexture, coord + vec2(-texel.x, -texel.y));
	color += texture(screenTexture, coord + vec2(0	   ,  texel.y));
    color += texture(screenTexture, coord); //middle
	color += texture(screenTexture, coord + vec2(0	   , -texel.y));
	color += texture(screenTexture, coord + vec2( texel.x,  texel.y));
	color += texture(screenTexture, coord + vec2( texel.x, 0));
	color += texture(screenTexture, coord + vec2( texel.x, -texel.y));
	
	return color / 9;
}

void main() {
    FragColor = boxBlur();
}