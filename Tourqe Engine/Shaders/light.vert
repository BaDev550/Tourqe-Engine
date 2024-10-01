#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 camMatrix;
uniform vec3 aPosM;

void main(){
	gl_Position = camMatrix * model * vec4(aPos + aPosM, 1.0f);
}