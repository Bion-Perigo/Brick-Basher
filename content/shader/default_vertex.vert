#version 330 core

in layout(location=0) vec3 position;
in layout(location=1) vec4 vertColor;
in layout(location=2) vec2 vertUVIn;
uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

out vec4 color;
out vec2 UV;

void main(){
	gl_Position = proj * view * model * vec4(position, 1.0f);
	
	color = vertColor;
	UV = vertUVIn;
}
