#version 330 core

uniform sampler2D mainTex;
uniform vec4 color;
uniform vec2 uv;
uniform vec2 frames;

in vec4 vertColor;
in vec2 texCoord;

out vec4 fragColor;

void main(){
	float x = 1 / frames.x;
	float y = 1 / frames.y;
	vec4 texColor = texture(mainTex, vec2(texCoord.x * x, texCoord.y * y) + vec2(x * uv.x, y * uv.y));
	if(texColor.a < 0.2)
		discard;
		
	fragColor = texColor * color * vertColor;
	
}
