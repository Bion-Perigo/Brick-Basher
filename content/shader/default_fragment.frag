#version 330 core

uniform sampler2D mainTex;

in vec4 color;
in vec2 UV;

out vec4 fragColor;

void main(){
	fragColor = color * texture(mainTex, UV);
}
