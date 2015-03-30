#version 410 core
layout(location = 0) in float sampleValue;

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;

void main()
{
	float x = bufferOffset + gl_VertexID;

	float y = y0 + yScale*sampleValue;

	gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
