#version 410 core

layout(location = 0) in float sampleValue;

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;

void main()
{
	vec4 tmp = vec4(bufferOffset + gl_VertexID, y0 + yScale*sampleValue, 0, 1);

	gl_Position	= transformMatrix*tmp;
}
