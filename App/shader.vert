#version 410 core

layout(location = 0) in float position;

uniform mat4 transformMatrix;
uniform float y0;
uniform int firstSample;
uniform float yScale;

void main()
{
	vec4 tmp = vec4(firstSample + gl_VertexID, y0 + yScale*position, 0, 1);

	gl_Position	= transformMatrix*tmp;
}
