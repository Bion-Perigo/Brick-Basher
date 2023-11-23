#version 330 core

uniform mat4 proj;
uniform mat4 model;

layout(location = 0) in vec3 vertPositionIn;
layout(location = 1) in vec4 vertColorIn;
layout(location = 2) in vec2 vertUVIn;

out vec4 vertColor;
out vec2 texCoord;

void main(){
	gl_Position = proj * model * vec4(vertPositionIn, 1.0f);
	vertColor = vertColorIn;
	texCoord = vertUVIn;
}
