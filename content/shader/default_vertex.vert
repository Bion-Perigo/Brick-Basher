#version 330 core

uniform mat4 proj;
uniform mat4 model;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec4 vertColor;
layout(location = 2) in vec2 vertUV;

out vec4 color;
out vec2 UV;

void main(){
	gl_Position = proj * model * vec4(vertPosition, 1.0f);
	color = vertColor;
	UV = vertUV;
}
