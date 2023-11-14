#version 330 core

in vec4 color;
in vec2 UV;

uniform sampler2D mainTex;

out vec4 fragColor;

void main(){
	fragColor = texture(mainTex, UV);
}
