#version 330 core

in layout(location=0) vec3 position;
in layout(location=1) vec4 vertColor;
uniform mat4 proj;
uniform vec4 myColor;
out vec4 color;

void main(){
	gl_Position= proj * vec4(position, 1.0f);
	
	color = myColor * vertColor;
}
