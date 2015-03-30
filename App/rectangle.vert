#version 410 core
layout(location = 0) in vec2 inputVertex;

uniform mat4 transformMatrix;

void main()
{
	gl_Position = transformMatrix*vec4(inputVertex, 0, 1);
}
