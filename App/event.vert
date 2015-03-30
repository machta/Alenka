#version 410 core
layout(location = 0) in float sampleValue;

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;
uniform float eventWidth;
uniform int divideBy;

void main()
{
	float x = bufferOffset + gl_VertexID/divideBy;

	float y = y0 + yScale*sampleValue + eventWidth*(1 - 2*(gl_VertexID & 1));

	gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
